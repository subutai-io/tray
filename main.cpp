#include <iostream>
#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QDir>
#include <QSplashScreen>

#include "IVBoxManager.h"
#include "TrayControlWindow.h"
#include "DlgLogin.h"
#include "TrayWebSocketServer.h"
#include "ApplicationLog.h"
#include "SettingsManager.h"
#include "updater/UpdaterComponentTray.h"

#include "rtm/include/RtmController.h"
////////////////////////////////////////////////////////////////////////////

/*!
 * \brief main - the entry point of SubutaiTray application
 * \param argc - argument count
 * \param argv - array of arguments
 * \return
 * arguments can be :
 * --v  - uses for getting version of tray application
 * --l  - uses to set log_level. can be 0, 1 and 2. 0 - most detailed. or use "trace", "info" and "error"
 */

int
main(int argc, char *argv[]) {

//  rtm::CRtmController::Instance()->Megatest();

  QApplication::setApplicationName("SubutaiTray");
  QApplication::setOrganizationName("subut.ai");
  QApplication app(argc, argv);

  QCommandLineParser cmd_parser;

  cmd_parser.setApplicationDescription("This tray application should help users to work with hub");
  QCommandLineOption log_level_opt("l",
                                   "Log level can be TRACE (0), INFO (1) and ERROR (2). Trace is most detailed logs.",
                                   "log_level",
                                   "info");
  QCommandLineOption version_opt("v",
                                 "Version",
                                 "Version");

  cmd_parser.addOption(log_level_opt);
  cmd_parser.addPositionalArgument("log_level", "Log level to use in this application");
  cmd_parser.addOption(version_opt);
  cmd_parser.addHelpOption();
  cmd_parser.parse(QApplication::arguments());

  CApplicationLog::Instance()->SetDirectory(
        CSettingsManager::Instance().logs_storage().toStdString());

  QString ll = cmd_parser.value(log_level_opt);
  if(ll == "trace" || ll == "0")
    CApplicationLog::Instance()->SetLogLevel(CApplicationLog::LT_TRACE);
  else if (ll == "info" || ll == "1")
    CApplicationLog::Instance()->SetLogLevel(CApplicationLog::LT_INFO);
  else if (ll == "error" || ll == "2")
    CApplicationLog::Instance()->SetLogLevel(CApplicationLog::LT_ERROR);

  if (cmd_parser.isSet(version_opt)) {
    std::cout << GIT_VERSION << std::endl;
    return 0;
  }
  CApplicationLog::Instance()->LogInfo("Tray application %s launched", GIT_VERSION);

  app.setQuitOnLastWindowClosed(false);
  qRegisterMetaType<notification_level_t>("notification_level_t");

  QString tmp[] = {".tmp", "_download"};
  for (int i = 0; i < 2; ++i) {
    QString tmp_file_path = QString(update_system::CUpdaterComponentTray::tray_kurjun_file_name()) + tmp[i];
    QFile tmp_file(tmp_file_path);
    if (tmp_file.exists()) {
      if (!tmp_file.remove()) {
        CApplicationLog::Instance()->LogError("Couldn't remove file %s", tmp_file_path.toStdString().c_str());
      }
    }
  }

  CRestWorker::Instance()->create_network_manager();

//  std::vector<CGorjunFileInfo> v =
//      CRestWorker::Instance()->get_gorjun_file_info("SubutaiTray");
//  for (auto i = v.begin(); i != v.end(); ++i)
//    CApplicationLog::Instance()->LogTrace("%s", i->id().toStdString().c_str());

  int result = 0;
  do {
    DlgLogin dlg;
    dlg.setModal(true);

    QPixmap pm(":/hub/tray_splash.png");
    QSplashScreen sc(pm);
    sc.show();

    dlg.run_dialog(&sc);
    if (dlg.result() == QDialog::Rejected)
      break;    

    CTrayServer::Instance()->Init();
    CVBoxManagerSingleton::Instance()->init_com();
    TrayControlWindow tcw;
    result = app.exec();
  } while (0);


  CRestWorker::Instance()->free_network_manager();
  return result;
}
////////////////////////////////////////////////////////////////////////////
