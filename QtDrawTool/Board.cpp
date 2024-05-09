#include "Board.h"

#include <QFileDialog>
#include <QPainter>
#include <QMouseEvent>

Board::Board(QWidget* parent): QWidget(parent)
{
    m_img.fill(Qt::black);
    setMouseTracking(true);
}

void Board::saveImg()
{
    QString filename =
        QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("*;;*.bmp;; *.png;; *.jpg;; *.tif;; *.GIF"));
    if (filename.isEmpty())
        return;
    else
        m_img.save(filename);
}

void Board::openImg()
{
    QString filename =
        QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("*;;*.bmp;; *.png;; *.jpg;; *.tif;; *.GIF"));
    if (filename.isEmpty())
        return;
    else
        m_img.load(filename);
}

void Board::undo()
{
    if (m_undos.empty()) return;
    if (m_redos.empty())
    {
        m_redos.push(std::move(m_undos.top()));
        m_undos.pop();
    }
    qDebug() << m_undos.size() << m_redos.size();
    if (m_undos.empty()) return;
    m_img = m_undos.top();
    m_redos.push(std::move(m_undos.top()));
    m_undos.pop();
    update();
}

void Board::redo()
{
    if (m_redos.empty()) return;
    if (m_undos.empty())
    {
        m_undos.push(std::move(m_redos.top()));
        m_redos.pop();
    }
    qDebug() << m_undos.size() << m_redos.size();
    if (m_redos.empty()) return;
    m_img = m_redos.top();
    m_undos.push(std::move(m_redos.top()));
    m_redos.pop();
    update();
}

void Board::reset()
{
    m_img.fill(Qt::black);
    update();
}

void Board::paint(QImage* img)
{
    QPainter painter(img);
    painter.setPen(m_pen);
    painter.setBrush(m_brush);

    auto width = m_end_pt.x() - m_start_pt.x();
    auto height = m_end_pt.y() - m_start_pt.y();

    switch (m_shape)
    {
    case Board::Shape::NONE:
        break;
    case Board::Shape::PIN:
        {
            auto font = painter.font();
            font.setPointSize(font.pointSize() * 5);
            painter.setFont(font);
            painter.setPen(QPen{Qt::blue, 5, Qt::SolidLine});
            // FIXME: almost at cursor pos, maybe use icon image is better
            painter.drawText(m_start_pt.x() - 15, m_start_pt.y() - 15, 30, 30, Qt::AlignHCenter | Qt::AlignVCenter,
                             "X");
            break;
        }
    case Board::Shape::RECT:
        painter.drawRect(m_start_pt.x(), m_start_pt.y(), width, height);
        break;
    case Board::Shape::CIRCLE:
        painter.drawEllipse(m_start_pt, width, height > 0 ? width : -width);
        break;
    case Board::Shape::ELLIPSE:
        painter.drawEllipse(m_start_pt.x(), m_start_pt.y(), width, height);
        break;
    case Board::Shape::POLYGON:
        qFatal() << "not implemented yet";
        break;
    default:
        break;
    }
    if (m_undos.empty() || (!m_mouse_pressing && m_undos.top() != m_img))
    {
        m_undos.push(m_img);
        while (!m_redos.empty())
            m_redos.pop();
        qDebug() << m_undos.size();
    }
    update();
}

void Board::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.drawImage(0, 0, m_drawing ? m_tmp_img : m_img);
}

void Board::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_start_pt = event->pos();
        m_drawing = true;
        m_mouse_pressing = true;
        if (m_shape == Shape::PIN)
        {
            m_tmp_img = m_img;
            paint(&m_tmp_img);
        }
    }
}

void Board::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        m_end_pt = event->pos();
        if (m_shape == Shape::NONE)
        {
            m_drawing = false;
            paint(&m_img);
        }
        else
        {
            m_tmp_img = m_img;
            paint(&m_tmp_img);
        }
    }
}

void Board::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_end_pt = event->pos();
        m_drawing = false;
        m_mouse_pressing = false;
        paint(&m_img);
    }
}