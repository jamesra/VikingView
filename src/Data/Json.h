#include <QScriptValue>
#include <QMap>

class Json
{
public:
  Json();
  ~Json();

  static QString encode( const QMap<QString, QVariant> &map );
  static QMap<QString, QVariant> decode( const QString &jsonStr );
  static QScriptValue encodeInner( const QMap<QString, QVariant> &map, QScriptEngine* engine );
  static QMap<QString, QVariant> decodeInner( QScriptValue object );
  static QList<QVariant> decodeInnerToList( QScriptValue arrayValue );
};
