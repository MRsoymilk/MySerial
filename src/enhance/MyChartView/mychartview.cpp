#include "mychartview.h"
#include "DraggableLine/draggableline.h"

bool MyChartView::isDraggingLine() const
{
    if (!scene())
        return false;
    for (QGraphicsItem *item : scene()->selectedItems()) {
        if (dynamic_cast<DraggableLine *>(item) || dynamic_cast<QGraphicsRectItem *>(item)) {
            return true;
        }
    }
    return false;
}
