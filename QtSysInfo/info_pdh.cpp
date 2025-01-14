#include <QReadWriteLock>
#include <QHash>
#include <QDebug>
#include <Pdh.h>
#include <PdhMsg.h>

#include <QApplication>
#include <QTimer>

#pragma comment(lib, "pdh.lib")

class PdhQuery
{
public:
    PdhQuery()
    {
        m_module = LoadLibrary(L"pdh.dll");
        if (auto status = PdhOpenQueryA(nullptr, 0, &m_query); status != ERROR_SUCCESS)
        {
            qWarning() << "Couldn't create Phd query, got error: " << formatPdhErrorMessage(status);
            throw std::exception();
        }
    }

    ~PdhQuery()
    {
        QWriteLocker lock(&m_rwLock);

        PDH_HCOUNTER counter;
        foreach (counter, m_counters.values())
        {
            PdhRemoveCounter(counter);
        }

        PdhCloseQuery(m_query);
    }

    void addEnglishCounter(const QString& key, LPCWSTR query)
    {
        QWriteLocker lock{&m_rwLock};
        if (m_counters.find(key) != m_counters.end()) throw std::exception("Key already exists");

        PDH_HCOUNTER counter;
        if (auto status = PdhAddEnglishCounterW(m_query, query, 0, &counter); status != ERROR_SUCCESS)
        {
            qWarning() << "Creating pdh counter result in error:" << formatPdhErrorMessage(status);
            throw std::exception("Couldn't create pdh counter");
        }
        m_counters.emplace(key, counter);
    }

    [[nodiscard]] double get(const QString& key) const
    {
        QReadLocker lock{const_cast<QReadWriteLock*>(&m_rwLock)};

        const auto it = m_counters.find(key);
        if (it == m_counters.end()) throw std::exception("Key doesn't exist");

        // https://stackoverflow.com/questions/77643749/getting-gpu-usage-statistics-with-pdh
        // https://learn.microsoft.com/en-us/windows/win32/api/pdh/nf-pdh-pdhgetformattedcounterarrayw
        if (key.contains("*"))
        {
            const PDH_HCOUNTER counter = *it;
            DWORD bufferSize = 0;
            DWORD itemCount = 0;
            PPDH_FMT_COUNTERVALUE_ITEM_W pItems = nullptr;

            auto status = PdhGetFormattedCounterArrayW(counter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, nullptr);
            if (status != PDH_MORE_DATA)
            {
                qWarning() << "Formatting counter result in error:" << formatPdhErrorMessage(status);
                throw std::exception("Couldn't format value");
            }

            pItems = (PPDH_FMT_COUNTERVALUE_ITEM_W)malloc(bufferSize);
            if (!pItems)
            {
                qWarning() << "PdhGetFormattedCounterArrayW failed to malloc buffer";
                throw std::exception("Failed to malloc buffer");
            }

            status = PdhGetFormattedCounterArrayW(counter, PDH_FMT_DOUBLE, &bufferSize, &itemCount, pItems);
            if (status != ERROR_SUCCESS)
            {
                qWarning() << "Formatting counter result in error:" << formatPdhErrorMessage(status);
                free(pItems);
                throw std::exception("Couldn't format value");
            }

            double res = 0;
            for (DWORD i = 0; i < itemCount; i++)
                res += pItems[i].FmtValue.doubleValue;
            free(pItems);
            return res;
        }

        const PDH_HCOUNTER counter = *it;
        PDH_FMT_COUNTERVALUE counterValue;
        if (auto status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &counterValue);
            status != ERROR_SUCCESS)
        {
            qWarning() << "Formatting counter result in error:" << formatPdhErrorMessage(status);
            throw std::exception("Couldn't format value");
        }
        return counterValue.doubleValue;
    }

    void refresh() const
    {
        QReadLocker lock{const_cast<QReadWriteLock*>(&m_rwLock)};
        PdhCollectQueryData(m_query);
    }

private:
    QString formatPdhErrorMessage(PDH_STATUS status) const
    {
        if (status == ERROR_SUCCESS) return QStringLiteral("");

        LPWSTR error;
        if (auto formatStatus = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                                  FORMAT_MESSAGE_IGNORE_INSERTS,
                                              m_module, status, 0, reinterpret_cast<LPWSTR>(&error), 0, nullptr);
            formatStatus == 0)
            return QStringLiteral("Failed to format error");

        if (!error) return QStringLiteral("");
        return QString::fromWCharArray(error);
    }

private:
    QReadWriteLock m_rwLock;
    PDH_HQUERY m_query = nullptr;
    QHash<QString, PDH_HCOUNTER> m_counters;
    HMODULE m_module = nullptr;
};

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    auto* timer = new QTimer();
    timer->setInterval(1000);

    auto* query = new PdhQuery();
    query->addEnglishCounter("total{T}Idle", L"\\Processor(_Total)\\% Idle Time");
    query->addEnglishCounter("total{T}Memory", L"\\Memory\\% Committed Bytes In Use");
    query->addEnglishCounter("total{T}AvailableMemory", L"\\Memory\\Available MBytes");
    query->addEnglishCounter("total{T}Disk", L"\\PhysicalDisk(_Total)\\% Disk Time");
    query->addEnglishCounter("total{T}DiskRead", L"\\PhysicalDisk(_Total)\\Disk Read Bytes/sec");
    query->addEnglishCounter("total{T}DiskWrite", L"\\PhysicalDisk(_Total)\\Disk Write Bytes/sec");

    // FIXME: this will mix multiple GPU cards' usage data...
    // i can not find any counter related to seperated nvida gpu in `perfmon.exe`...
    // may be the only solid way is through nvapi like [this](https://eliang.blogspot.com/2011/05/getting-nvidia-gpu-usage-in-c.html)?
    query->addEnglishCounter("total{*}GPU", L"\\GPU Engine(*)\\Utilization Percentage");
    query->addEnglishCounter("total{*}GPUMemory", L"\\GPU Process Memory(*)\\Dedicated Usage");
    query->refresh();

    QObject::connect(timer, &QTimer::timeout, [query]() {
        query->refresh();
        qDebug() << "CPU:" << 100.0 - query->get("total{T}Idle") << "%";
        qDebug() << "Memory Commited Percentage:" << query->get("total{T}Memory") << "%";
        qDebug() << "Memory Available:" << query->get("total{T}AvailableMemory") << "MB";
        qDebug() << "Disk:" << query->get("total{T}Disk") << "%";
        qDebug() << "Disk Read:" << query->get("total{T}DiskRead") / 1024 << "KB/s";
        qDebug() << "Disk Write:" << query->get("total{T}DiskWrite") / 1024 << "KB/s";
        qDebug() << "GPU:" << query->get("total{*}GPU") << "%";
        qDebug() << "GPU Memory:" << query->get("total{*}GPUMemory") / 1024 / 1024 << "MB";
    });
    timer->start();

    return a.exec();
}