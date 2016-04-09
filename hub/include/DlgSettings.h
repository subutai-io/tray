#ifndef DLGSETTINGS_H
#define DLGSETTINGS_H

#include <QDialog>
#include <QTabWidget>
#include <QEvent>

namespace Ui {
  class DlgSettings;
}

class TabResizeFilter : public QObject {
private:
  QTabWidget* m_target;

  static void expandingTypeStyleSheet(QTabWidget* tw) {
    tw->setStyleSheet(QString("QTabBar::tab:!selected {width : %1px;}"
                              "QTabBar::tab:selected {width : %2px;}").
                      arg(tw->width() / tw->count()-1).
                      arg(tw->width() / tw->count()-1));
  }

public:
  TabResizeFilter(QTabWidget* target) : QObject(target), m_target(target) {}
  bool eventFilter(QObject *, QEvent *ev) {
    if (ev->type() == QEvent::Resize)
      expandingTypeStyleSheet(m_target);
    return false;
  }
};
////////////////////////////////////////////////////////////////////////////

class DlgSettings : public QDialog
{
  Q_OBJECT

public:
  explicit DlgSettings(QWidget *parent = 0);
  ~DlgSettings();

private:
  Ui::DlgSettings *ui;
  TabResizeFilter *m_tab_resize_filter;

private slots:
  void btn_ok_released();
  void btn_cancel_released();
  void btn_terminal_file_dialog_released();
  void btn_p2p_file_dialog_released();
};

#endif // DLGSETTINGS_H
