#include "axistag.h"

// Kód v tomto souboru je převzatý z článku
// "Creating dynamic axis tags using items" v návodu k QCustomPlot

// https://www.qcustomplot.com/index.php/tutorials/specialcases/axistags

AxisTag::AxisTag(QCPAxis *parentAxis) : QObject(parentAxis), mAxis(parentAxis) {
  // The dummy tracer serves here as an invisible anchor which always sticks to the right side of
  // the axis rect
  mDummyTracer = new QCPItemTracer(mAxis->parentPlot());
  mDummyTracer->setVisible(false);
  mDummyTracer->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
  mDummyTracer->position->setTypeY(QCPItemPosition::ptPlotCoords);
  mDummyTracer->position->setAxisRect(mAxis->axisRect());
  mDummyTracer->position->setAxes(0, mAxis);
  mDummyTracer->position->setCoords(1, 0);

  // the arrow end (head) is set to move along with the dummy tracer by setting it as its parent
  // anchor. Its coordinate system (setCoords) is thus pixels, and this is how the needed horizontal
  // offset for the tag of the second y axis is achieved. This horizontal offset gets dynamically
  // updated in AxisTag::updatePosition. the arrow "start" is simply set to have the "end" as parent
  // anchor. It is given a horizontal offset to the right, which results in a 15 pixel long arrow.
  mArrow = new QCPItemLine(mAxis->parentPlot());
  mArrow->setLayer("overlay");
  mArrow->setClipToAxisRect(false);
  mArrow->setHead(QCPLineEnding::esSpikeArrow);
  mArrow->end->setParentAnchor(mDummyTracer->position);
  mArrow->start->setParentAnchor(mArrow->end);
  mArrow->start->setCoords(15, 0);

  // The text label is anchored at the arrow start (tail) and has its "position" aligned at the
  // left, and vertically centered to the text label box.
  mLabel = new QCPItemText(mAxis->parentPlot());
  mLabel->setLayer("overlay");
  mLabel->setClipToAxisRect(false);
  mLabel->setPadding(QMargins(3, 0, 3, 0));
  mLabel->setBrush(QBrush(Qt::white));
  mLabel->setPen(QPen(Qt::blue));
  mLabel->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  mLabel->position->setParentAnchor(mArrow->start);
}

AxisTag::~AxisTag() {
  if (mDummyTracer)
    mDummyTracer->parentPlot()->removeItem(mDummyTracer);
  if (mArrow)
    mArrow->parentPlot()->removeItem(mArrow);
  if (mLabel)
    mLabel->parentPlot()->removeItem(mLabel);
}

void AxisTag::setPen(const QPen &pen) {
  mArrow->setPen(pen);
  mLabel->setPen(pen);
}

void AxisTag::setBrush(const QBrush &brush) { mLabel->setBrush(brush); }

void AxisTag::setText(const QString &text) { mLabel->setText(text); }

void AxisTag::updatePosition(double value) {
  // since both the arrow and the text label are chained to the dummy tracer (via anchor
  // parent-child relationships) it is sufficient to update the dummy tracer coordinates. The
  // Horizontal coordinate type was set to ptAxisRectRatio so to keep it aligned at the right side
  // of the axis rect, it is always kept at 1. The vertical coordinate type was set to
  // ptPlotCoordinates of the passed parent axis, so the vertical coordinate is set to the new
  // value.
  mDummyTracer->position->setCoords(1, value);

  // We want the arrow head to be at the same horizontal position as the axis backbone, even if
  // the axis has a certain offset from the axis rect border (like the added second y axis). Thus we
  // set the horizontal pixel position of the arrow end (head) to the axis offset (the pixel
  // distance to the axis rect border). This works because the parent anchor of the arrow end is
  // the dummy tracer, which, as described earlier, is tied to the right axis rect border.
  mArrow->end->setCoords(mAxis->offset(), 0);
}
