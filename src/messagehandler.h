#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QAbstractMessageHandler>

class MessageHandler final : public QAbstractMessageHandler
{
public:
    MessageHandler()
        : QAbstractMessageHandler(nullptr)
    {
    }

    QString statusMessage() const
    {
        return m_description;
    }

    qint64 line() const
    {
        return m_sourceLocation.line();
    }

    qint64 column() const
    {
        return m_sourceLocation.column();
    }

protected:
    virtual void handleMessage(
        QtMsgType type,
        const QString &description,
        const QUrl &identifier,
        const QSourceLocation &sourceLocation)
    {
        Q_UNUSED(type);
        Q_UNUSED(identifier);

        m_description = description;
        m_sourceLocation = sourceLocation;
    }

private:
    QString m_description;
    QSourceLocation m_sourceLocation;
};

#endif // MESSAGEHANDLER_H
