#include "updatechecker.h"

#include <QNetworkRequest>
#include <QDesktopServices>

#ifdef Q_OS_ANDROID
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#endif

const char *GooglePlayURL = "https://play.google.com/store/apps/details?id=";
const char *URLLanguage = "&hl=en";

#ifdef Q_OS_ANDROID
QString jstringToQString(jstring string)
{
    QAndroidJniEnvironment env;
    jboolean jfalse = false;
    return QString(env->GetStringUTFChars(string, &jfalse));
}
#endif

UpdateChecker::UpdateChecker(QObject *parent) :
    QObject(parent)
{
    m_netAccess = new QNetworkAccessManager(this);
    connect(m_netAccess, SIGNAL(finished(QNetworkReply*)), SLOT(networkRequestFinished(QNetworkReply*)));

    m_packageName = retrievePackageName();
    m_version = retrieveVersion();
}

QString UpdateChecker::retrievePackageName()
{
#ifdef Q_OS_ANDROID
    QAndroidJniEnvironment env;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity",
                                                                           "()Landroid/app/Activity;");
    jclass activityClass = env->GetObjectClass(activity.object<jobject>());

    jmethodID mIDGetPackageName = env->GetMethodID(activityClass,
                                                   "getPackageName",
                                                   "()Ljava/lang/String;");

    jstring packageName = (jstring)env->CallObjectMethod(activity.object<jobject>(), mIDGetPackageName);

    return jstringToQString(packageName);
#else
    return "";
#endif
}

QString UpdateChecker::retrieveVersion()
{
#ifdef Q_OS_ANDROID
    QAndroidJniEnvironment env;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative",
                                                                           "activity",
                                                                           "()Landroid/app/Activity;");
    jclass activityClass = env->GetObjectClass(activity.object<jobject>());

    jmethodID mIDGetPackageName = env->GetMethodID(activityClass,
                                                   "getPackageName",
                                                   "()Ljava/lang/String;");

    jstring packageName = (jstring)env->CallObjectMethod(activity.object<jobject>(), mIDGetPackageName);

    jmethodID mIDGetPackageManager = env->GetMethodID(activityClass, "getPackageManager","()Landroid/content/pm/PackageManager;");

    jobject packageManager = env->CallObjectMethod(activity.object<jobject>(), mIDGetPackageManager);

    jclass packageManagerClass = env->GetObjectClass(packageManager);

    jmethodID mIDGetPackageInfo = env->GetMethodID(packageManagerClass,
                                                   "getPackageInfo",
                                                   "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");

    jobject packageInfo = env->CallObjectMethod(packageManager, mIDGetPackageInfo, packageName);
    jclass packageInfoClass = env->GetObjectClass(packageInfo);

    jfieldID fIDVersionName = env->GetFieldID(packageInfoClass, "versionName", "Ljava/lang/String;");
    jstring versionName = (jstring)env->GetObjectField(packageInfo, fIDVersionName);

    return jstringToQString(versionName);
#else
    return "";
#endif
}

void UpdateChecker::checkForUpdateOnGooglePlay()
{
    if (!m_packageName.isEmpty()) {
        QUrl url(GooglePlayURL + m_packageName + URLLanguage);
        QNetworkRequest request(url);
        m_netAccess->get(request);
    }
}

void UpdateChecker::openPackageOnGooglePlay()
{
    if (!m_packageName.isEmpty())
        QDesktopServices::openUrl("market://details?id=" + m_packageName);
}

QString UpdateChecker::packageName() const
{
    return m_packageName;
}

void UpdateChecker::setPackageName(const QString &packageName)
{
    if (m_packageName == packageName)
        return;

    m_packageName = packageName;
    emit packageNameChanged();
}

QString UpdateChecker::version() const
{
    return m_version;
}

void UpdateChecker::setVersion(const QString &version)
{
    if (m_version == version)
        return;

    m_version = version;
    emit versionChanged();
}

QString UpdateChecker::latestVersion() const
{
    return m_latestVersion;
}

void UpdateChecker::setLatestVersion(const QString &latestVersion)
{
    if (m_latestVersion == latestVersion)
        return;

    m_latestVersion = latestVersion;
    emit latestVersionChanged();
}

void UpdateChecker::networkRequestFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit errorDownloading();
        reply->deleteLater();
        return;
    }

    QVariant redir = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (redir.isValid()) {
        QUrl url = redir.toUrl();
        if (url.isRelative())
            url.setScheme(reply->url().scheme());

        QNetworkRequest request(url);
        m_netAccess->get(request);
        reply->deleteLater();
        return;
    }

    QString html(reply->readAll());
    QString keyString = "softwareVersion\">";
    html = html.mid(html.indexOf(keyString) + keyString.length());
    html = html.mid(0, html.indexOf("</")).simplified();

    if (!html.isEmpty())
        setLatestVersion(html);
}