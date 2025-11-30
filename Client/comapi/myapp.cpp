#include "myapp.h"
#include <QDesktopWidget>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QDebug>
#include <QSettings>

// 应用程序配置目录
QString MyApp::m_strAppPath         = "./";
QString MyApp::m_strDataPath        = "";
QString MyApp::m_strRecvPath        = "";
QString MyApp::m_strDatabasePath    = "";
QString MyApp::m_strConfPath        = "";
QString MyApp::m_strFacePath        = "";
QString MyApp::m_strHeadPath        = "";
QString MyApp::m_strSoundPath       = "";
QString MyApp::m_strRecordPath      = "";

// 配置文件
QString MyApp::m_strIniFile         = "config.ini";

// 服务器相关配置
QString MyApp::m_strHostAddr        = "10.2.229.121";
int     MyApp::m_nMsgPort           = 60101;
int     MyApp::m_nFilePort          = 60102;
int     MyApp::m_nGroupPort         = 60103;

QString MyApp::m_strUserName        = "bxz";
QString MyApp::m_strPassword        = "111";
QString MyApp::m_strHeadFile        = "head-64.png";

int     MyApp::m_nId                = -1;
int     MyApp::m_nWinX              = 0;
int     MyApp::m_nWinY              = 0;

void MyApp::InitApp(const QString &appPath)
{
    m_strAppPath        = appPath + "/";

        m_strDataPath       = m_strAppPath  + "Data/";
        m_strRecvPath       = m_strDataPath + "RecvFiles/";
        m_strDatabasePath   = m_strDataPath + "Database/";
        m_strConfPath       = m_strDataPath + "Conf/";
        m_strHeadPath       = m_strDataPath + "Head/";
        m_strSoundPath      = m_strDataPath + "Sound/";
        m_strRecordPath     = m_strDataPath + "Record/";
        m_strFacePath       = m_strDataPath + "Face/";
        m_strIniFile        = m_strConfPath + "config.ini";

        // 检查目录
        CheckDirs();

        // 检测音频文件
        CheckSound();

        // 创建配置文件
        CreatorSettingFile();

        // 加载系统配置
        ReadSettingFile();
}

void MyApp::CreatorSettingFile()
{

}

void MyApp::ReadSettingFile()
{

}

void MyApp::SetSettingFile(const QString &group, const QString &key, const QVariant &value)
{

}

QVariant MyApp::GetSettingKeyValue(const QString &group, const QString &key, const QVariant &value)
{

}

/**
 * @brief MyApp::CheckDirs
 * 检查文件是否存在
 */
void MyApp::CheckDirs()
{
    //数据文件夹
    QDir dir(m_strDataPath);
    if(!dir.exists()){
        dir.mkdir(m_strDataPath);
    }

    //接收文件
    dir.setPath(m_strRecvPath);
    if(!dir.exists()){
        dir.mkdir(m_strRecvPath);
    }

    //数据库
    dir.setPath(m_strDatabasePath);
    if(!dir.exists()){
        dir.mkdir(m_strDatabasePath);
    }


    // 配置文件目录
    dir.setPath(m_strConfPath);
    if (!dir.exists()) {
        dir.mkdir(m_strConfPath);
    }

    // 表情目录
    dir.setPath(m_strFacePath);
    if (!dir.exists()) {
        dir.mkdir(m_strFacePath);
    }

    // 头像检测目录
    dir.setPath(m_strHeadPath);
    if (!dir.exists()) {
        dir.mkdir(m_strHeadPath);
    }

    // 音频目录
    dir.setPath(m_strSoundPath);
    if (!dir.exists()) {
        dir.mkdir(m_strSoundPath);
    }
}

void MyApp::CheckSound()
{
    if(!QFile::exists(MyApp::m_strSoundPath + "message.wav")){
        QFile::copy(":/sound/resource/sound/message.wav", MyApp::m_strSoundPath + "message.wav");
    }
    if(!QFile::exists(MyApp::m_strSoundPath + "msg.wav")){
        QFile::copy(":/sound/resource/sound/msg.wav", MyApp::m_strSoundPath + "msg.wav");
    }
    if(!QFile::exists(MyApp::m_strSoundPath + "ringin.wav")){
        QFile::copy(":/sound/resource/sound/ringin.wav", MyApp::m_strSoundPath + "ringin.wav");
    }
    if(!QFile::exists(MyApp::m_strSoundPath + "system.wav")){
        QFile::copy(":/sound/resource/sound/system.wav", MyApp::m_strSoundPath + "system.wav");
    }
    if(!QFile::exists(MyApp::m_strSoundPath + "userlogon.wav")){
        QFile::copy(":/sound/resource/sound/userlogon.wav", MyApp::m_strSoundPath + "userlogon.wav");
    }

}

void MyApp::SaveConfig()
{
    QSettings settings(m_strIniFile,QSettings::IniFormat);

    // 保存用户信息
    settings.beginGroup("UserConfig");
    settings.setValue("userName",m_strUserName);
    settings.setValue("password",m_strPassword);
    settings.endGroup();

    //保存服务器信息
    settings.beginGroup("Server");
    settings.setValue("ServerIP", m_strHostAddr);
    settings.setValue("MsgPort",  m_nMsgPort);
    settings.setValue("FilePort",  m_nFilePort);
    settings.setValue("GroupPort",  m_nGroupPort);
    settings.endGroup();
    settings.sync();
}
