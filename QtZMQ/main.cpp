#include "ZMQClient.h"

#include <QApplication>
#include <QTimer>

#include <iostream>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    ZMQClient client{};
    client.set_address("tcp://localhost:5570");
    QObject::connect(&client, &ZMQClient::ready_to_read,
                     [&]() { std::cout << client.receive()[0].data() << std::endl; });
    QTimer timer(&client);
    int cnt = 0;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        auto sent = client.send({"hello"});
        assert(sent);

        if (cnt++ == 10)
        {
            timer.stop();
            app.quit();
        }
    });
    timer.start(100);

    return app.exec();
}
