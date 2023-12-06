#pragma once

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>

class QGraphicsView;

class QtDisplayBoard: public QWidget
{
    Q_OBJECT

public:
    explicit QtDisplayBoard(QWidget* parent = nullptr);
    ~QtDisplayBoard();

    [[nodiscard]] QSize sizeHint() const override;
    void resizeEvent(QResizeEvent* event) override;

    [[nodiscard]] QGraphicsVideoItem* getVideoItem() const;
    [[nodiscard]] QGraphicsScene* getScene() const;

private:
    QGraphicsVideoItem* m_video_item = nullptr;
    QGraphicsScene* m_scene = nullptr;
    QGraphicsView* m_view = nullptr;
};