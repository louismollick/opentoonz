

#include "histogrampopup.h"

// Tnz6 includes
#include "menubarcommandids.h"
#include "tapp.h"
#include "previewer.h"
#include "sceneviewer.h"

// TnzQt includes
#include "toonzqt/menubarcommand.h"
#include "toonzqt/combohistogram.h"

// TnzLib includes
#include "toonz/tframehandle.h"

// TnzCore includes
#include "trasterimage.h"
#include "tvectorimage.h"
#include "ttoonzimage.h"

// Qt includes
#include <QTimer>
#include <QMainWindow>
#include <QDesktopWidget>
#include <QFocusEvent>

using namespace DVGui;

//=============================================================================
/*! \class HistogramPopup
                \brief The HistogramPopup class provides a dialog to show an
   histogram \b Histogram

                Inherits \b Dialog.
*/
//-----------------------------------------------------------------------------

HistogramPopup::HistogramPopup(QString title)
    : QDialog(TApp::instance()->getMainWindow()) {
  setTitle(title);

  m_histogram = new ComboHistogram(this);

  QVBoxLayout *mainLay = new QVBoxLayout();
  mainLay->setMargin(0);
  mainLay->setSpacing(0);
  { mainLay->addWidget(m_histogram); }
  setLayout(mainLay);
  mainLay->setSizeConstraint(QLayout::SetFixedSize);
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

//-----------------------------------------------------------------------------

void HistogramPopup::setTitle(QString title) { setWindowTitle(title); }

//-----------------------------------------------------------------------------

void HistogramPopup::setImage(TImageP image) {
  TRasterImageP rimg = (TRasterImageP)image;
  TVectorImageP vimg = (TVectorImageP)image;
  TToonzImageP timg  = (TToonzImageP)image;

  TPaletteP palette;

  TRasterP ras;
  if (rimg)
    ras = rimg->getRaster();
  else if (timg) {
    ras     = timg->getRaster();
    palette = timg->getPalette();
  } else if (vimg)
    ras = vimg->render(false);

  m_histogram->setRaster(ras, palette);
}
//-----------------------------------------------------------------------------
/*! show the picked color
 */
void HistogramPopup::updateInfo(const TPixel32 &pix, const TPointD &imagePos) {
  m_histogram->updateInfo(pix, imagePos);
}

void HistogramPopup::updateInfo(const TPixel64 &pix, const TPointD &imagePos) {
  m_histogram->updateInfo(pix, imagePos);
}

//-----------------------------------------------------------------------------
/*! show the average-picked color
 */
void HistogramPopup::updateAverageColor(const TPixel32 &pix) {
  m_histogram->updateAverageColor(pix);
}

void HistogramPopup::updateAverageColor(const TPixel64 &pix) {
  m_histogram->updateAverageColor(pix);
}
//-----------------------------------------------------------------------------

void HistogramPopup::setShowCompare(bool on) {
  m_histogram->setShowCompare(on);
}

//-----------------------------------------------------------------------------

void HistogramPopup::invalidateCompHisto() {
  m_histogram->invalidateCompHisto();
}

//-----------------------------------------------------------------------------

void HistogramPopup::moveNextToWidget(QWidget *widget) {
  if (!widget) return;
  const int margin = 5;

  if (minimumSize().isEmpty()) grab();
  QSize popupSize = frameSize();

  int currentScreen = QApplication::desktop()->screenNumber(widget);
  QRect screenRect  = QApplication::desktop()->availableGeometry(currentScreen);
  QRect viewerRect  = widget->rect();
  viewerRect.moveTo(widget->mapToGlobal(QPoint(0, 0)));
  // decide which side to open the popup
  QPoint popupPos = widget->mapToGlobal(QPoint(0, 0));
  // open at the left
  if (viewerRect.left() - screenRect.left() >
      screenRect.right() - viewerRect.right())
    popupPos.setX(std::max(viewerRect.left() - popupSize.width() - margin, 0));
  // open at the right
  else
    popupPos.setX(std::min(viewerRect.right() + margin,
                           screenRect.right() - popupSize.width()));
  // adjust vertical position
  popupPos.setY(std::min(std::max(popupPos.y(), screenRect.top()),
                         screenRect.bottom() - popupSize.height() - margin));
  move(popupPos);
}

//=============================================================================
/*! \class ViewerHistogramPopup
                \brief The ViewerHistogramPopup show the histogram pertain to
   current viewer.

                Inherits \b Dialog.
*/
//-----------------------------------------------------------------------------

ViewerHistogramPopup::ViewerHistogramPopup()
    : HistogramPopup(tr("Viewer Histogram")) {}

//-----------------------------------------------------------------------------

void ViewerHistogramPopup::showEvent(QShowEvent *e) {
  connect(TApp::instance()->getCurrentFrame(), SIGNAL(frameSwitched()),
          SLOT(setCurrentRaster()));
  connect(Previewer::instance(), SIGNAL(activedChanged()),
          SLOT(setCurrentRaster()));

  setCurrentRaster();
  moveNextToWidget(TApp::instance()->getActiveViewer());
}

//-----------------------------------------------------------------------------

void ViewerHistogramPopup::hideEvent(QHideEvent *e) {
  disconnect(TApp::instance()->getCurrentFrame(), SIGNAL(frameSwitched()), this,
             SLOT(setCurrentRaster()));
  disconnect(Previewer::instance(), SIGNAL(activedChanged()), this,
             SLOT(setCurrentRaster()));
}

//-----------------------------------------------------------------------------

void ViewerHistogramPopup::setCurrentRaster() {
  TApp *app            = TApp::instance();
  Previewer *previewer = Previewer::instance();
  TRasterP ras         = 0;
  if (previewer->isActive()) {
    int currentFrame = app->getCurrentFrame()->getFrameIndex();
    // Se il preview del frame non e' pronto richiamo questo metodo dopo un
    // intervallo di 10 ms
    if (!previewer->isFrameReady(currentFrame)) {
      QTimer::singleShot(10, this, SLOT(setCurrentRaster()));
      return;
    }
    ras = previewer->getRaster(currentFrame);
  }
  m_histogram->setRaster(ras);
  update();
}

//=============================================================================

OpenPopupCommandHandler<ViewerHistogramPopup> openHistogramPopup(
    MI_ViewerHistogram);
