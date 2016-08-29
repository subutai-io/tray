#include <QThread>

#include "DlgAbout.h"
#include "ui_DlgAbout.h"
#include "SystemCallWrapper.h"
#include "SettingsManager.h"
#include "updater/HubComponentsUpdater.h"

using namespace update_system;

DlgAbout::DlgAbout(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::DlgAbout)
{
  ui->setupUi(this);
  ui->lbl_tray_version_val->setText(GIT_VERSION);

  //connect
  connect(ui->btn_p2p_update, SIGNAL(released()), this, SLOT(btn_p2p_update_released()));
  connect(ui->btn_tray_update, SIGNAL(released()), this, SLOT(btn_tray_update_released()));
  connect(ui->btn_rh_update, SIGNAL(released()), this, SLOT(btn_rh_update_released()));

  connect(CHubComponentsUpdater::Instance(), SIGNAL(download_file_progress(QString,qint64,qint64)),
          this, SLOT(download_progress(QString,qint64,qint64)));
  connect(CHubComponentsUpdater::Instance(), SIGNAL(update_available(QString)),
          this, SLOT(update_available(QString)));
  connect(CHubComponentsUpdater::Instance(), SIGNAL(updating_finished(QString,bool)),
          this, SLOT(update_finished(QString,bool)));

  m_dct_fpb[IUpdaterComponent::P2P] = {ui->pb_p2p, ui->btn_p2p_update};
  m_dct_fpb[IUpdaterComponent::TRAY] = {ui->pb_tray, ui->btn_tray_update};
  m_dct_fpb[IUpdaterComponent::RH] = {ui->pb_rh, ui->btn_rh_update};
  ui->pb_initialization_progress->setMaximum(DlgAboutInitializer::COMPONENTS_COUNT);

  QThread* th = new QThread;
  DlgAboutInitializer* di = new DlgAboutInitializer;
  connect(di, SIGNAL(finished()), th, SLOT(quit()), Qt::DirectConnection);
  connect(di, SIGNAL(finished()), this, SLOT(initialization_finished()), Qt::DirectConnection);

  connect(th, SIGNAL(started()), di, SLOT(do_initialization()));
  connect(di, SIGNAL(got_chrome_version(QString)), this, SLOT(got_chrome_version_sl(QString)));
  connect(di, SIGNAL(got_p2p_version(QString)), this, SLOT(got_p2p_version_sl(QString)));
  connect(di, SIGNAL(got_rh_version(QString)), this, SLOT(got_rh_version_sl(QString)));
  connect(di, SIGNAL(got_vbox_version(QString)), this, SLOT(got_vbox_version_sl(QString)));

  connect(di, SIGNAL(update_available(QString,bool)), this, SLOT(update_available_sl(QString,bool)));
  connect(di, SIGNAL(init_progress(int,int)), this, SLOT(init_progress_sl(int,int)));

  connect(th, SIGNAL(finished()), di, SLOT(deleteLater()));
  connect(th, SIGNAL(finished()), th, SLOT(deleteLater()));

  connect(this, SIGNAL(finished(int)),
          di, SLOT(abort()));

  di->moveToThread(th);
  th->start();

}

DlgAbout::~DlgAbout() {
  delete ui;
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::btn_tray_update_released() {
  CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::TRAY);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::btn_p2p_update_released() {
  CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::P2P);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::btn_rh_update_released() {
  CHubComponentsUpdater::Instance()->force_update(IUpdaterComponent::RH);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::download_progress(QString file_id,
                            qint64 rec,
                            qint64 total) {
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].pb->setValue((rec*100)/total);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::update_available(QString file_id) {
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].btn->setEnabled(true);
  m_dct_fpb[file_id].pb->setEnabled(true);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::update_finished(QString file_id,
                          bool success) {
  (void)success;
  if (m_dct_fpb.find(file_id) == m_dct_fpb.end()) return;
  m_dct_fpb[file_id].btn->setEnabled(false);
  m_dct_fpb[file_id].pb->setEnabled(false);
  m_dct_fpb[file_id].pb->setValue(0);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::initialization_finished() {
  ui->lbl_about_init->setEnabled(false);
  ui->pb_initialization_progress->setEnabled(false);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::init_progress_sl(int part,
                           int total) {
  (void) total; //right now we don't need to use it.
  ui->pb_initialization_progress->setValue(part);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::got_p2p_version_sl(QString version) {
  ui->lbl_p2p_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::got_chrome_version_sl(QString version) {
  ui->lbl_chrome_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::got_vbox_version_sl(QString version) {
  ui->lbl_vbox_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::got_rh_version_sl(QString version) {
  ui->lbl_rh_version_val->setText(version);
}
////////////////////////////////////////////////////////////////////////////

void
DlgAbout::update_available_sl(QString component_id,
                              bool available) {
  auto item = m_dct_fpb.find(component_id);
  if (item == m_dct_fpb.end()) return;
  item->second.pb->setEnabled(available);
  item->second.btn->setEnabled(available);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void
DlgAboutInitializer::do_initialization() {
  int initialized_component_count = 0;
  std::string str_version;
  CSystemCallWrapper::p2p_version(str_version);
  QString p2p_version = QString::fromStdString(str_version);
  emit got_p2p_version(p2p_version);
  emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

  CSystemCallWrapper::chrome_version(str_version);
  QString chrome_version = QString::fromStdString(str_version);
  emit got_chrome_version(chrome_version);
  emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

  QString vbox_version = CSystemCallWrapper::virtual_box_version();
  emit got_vbox_version(vbox_version);
  emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

  QString rh_version = CSystemCallWrapper::rh_version();
  emit got_rh_version(rh_version);
  emit init_progress(++initialized_component_count, COMPONENTS_COUNT);

  QString uas[] = {
    IUpdaterComponent::P2P, IUpdaterComponent::TRAY,
    IUpdaterComponent::RH, ""};

  for (int i = 0; uas[i] != ""; ++i) {
    bool ua = CHubComponentsUpdater::Instance()->is_update_available(uas[i]);
    emit update_available(uas[i], ua);
    emit init_progress(++initialized_component_count, COMPONENTS_COUNT);
  }

  emit finished();
}
////////////////////////////////////////////////////////////////////////////

void
DlgAboutInitializer::abort() {
  emit finished();
}
