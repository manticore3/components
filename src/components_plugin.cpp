#include "components_plugin.h"

#include <qqml.h>

#include "connectivitymanager.h"
#include "updatechecker.h"
#include "screenvalues.h"

static QObject *update_checker_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    UpdateChecker *updateChecker = new UpdateChecker();
    return updateChecker;
}

static QObject *screen_values_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    ScreenValues *screenValues = new ScreenValues();
    return screenValues;
}

void ComponentsPlugin::registerTypes(const char *uri)
{
    // @uri com.iktwo.components
    qmlRegisterType<ConnectivityManager>(uri, 1, 0, "ConnectivityManager");
    qmlRegisterSingletonType<UpdateChecker>(uri, 1, 0, "UpdateChecker", update_checker_provider);
    qmlRegisterSingletonType<ScreenValues>(uri, 1, 0, "ScreenValues", screen_values_provider);
}
