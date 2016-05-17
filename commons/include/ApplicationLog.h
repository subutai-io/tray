#ifndef APPLICATION_LOG_H
#define APPLICATION_LOG_H

#include <stdio.h>
#include "EventLoop.h"
#include "FunctorWithResult.h"
#ifndef RT_OS_WINDOWS
#include "commons/include/MRE_Linux.h"
#else
#include "commons/include/MRE_Windows.h"
#endif

/*!
 * \brief The CApplicationLog class. Uses for logging.
 * It generates trace_($current_date).txt, info_($current_date).txt and error_($current_date).txt
 * You can specify log_level via application argument --l
 */
class CApplicationLog
{
public:
  /*!
   * \brief The LOG_TYPE enum - log level enum
   */
  enum LOG_TYPE {
    LT_TRACE = 0,
    LT_INFO,
    LT_ERROR,
    LT_LAST
  };

  /*!
   * \brief Set directory where log files will be generated
   * \param directory - path to directory.
   */
  void SetDirectory(const char *directory);
  /*!
   * \brief Set log level.
   * \param lt - log level. See LOG_TYPE enum.
   */
  void SetLogLevel(LOG_TYPE lt) {m_log_level = lt;}

  void LogTrace( const char* format, ... );
  void LogInfo ( const char* format, ... );
  void LogError( const char* format, ... );

  static CApplicationLog* Instance(){
    static CApplicationLog m_instance;
    return &m_instance;
  }

private:
  static const int BUFFER_SIZE = 2048;
  static const char* LOG_FILE_DELIMITER;

  char m_messageBuffer[BUFFER_SIZE];
  std::string m_directory;


  std::string m_lst_files_by_log_type[LT_LAST];
#ifndef RT_OS_WINDOWS
  CEventLoop<SynchroPrimitives::CLinuxManualResetEvent> *m_logEventLoop;
#else
  CEventLoop<SynchroPrimitives::CWindowsManualResetEvent> *m_logEventLoop;
#endif
  LOG_TYPE m_log_level;

  CApplicationLog(void);
  ~CApplicationLog(void);

  CApplicationLog(const CApplicationLog&);
  void operator=(const CApplicationLog&);

  static int AppendLog(const char *str, std::string &logFileName);
  void UpdateLogFilesNames(void);  
  void Log(CApplicationLog::LOG_TYPE log_type, std::string msg);

public:  
};

#endif //APPLICATION_LOG_H
