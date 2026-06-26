//
// Created by niko on 14/05/2026.
//

#ifndef OPA_AI_UTILS_H
#define OPA_AI_UTILS_H

#include <QFuture>

/**
 * Reads a value from the keychain asynchronously.
 *
 * @param service The service name in the keychain.
 * @param key The key to retrieve.
 * @param parent The parent object for the future.
 *
 * @return A future that resolves to the retrieved value or a KeychainException.
 */
QFuture<QString> readFromKeychain(const QString& service, const QString& key, QObject* parent);

/**
 * Reads a value from the keychain asynchronously that can be empty.
 *
 * @param service The service name in the keychain.
 * @param key The key to retrieve.
 * @param parent The parent object for the future.
 *
 * @return A future that resolves to the retrieved value or a KeychainException.
 */
QFuture<QString> readFromKeychainLax(const QString& service, const QString& key, QObject* parent);

class KeychainException : public QException {
public:
    explicit KeychainException(const QString& message) : m_message(message), m_byteArray(message.toUtf8()) {
    }

    void raise() const override {
        throw *this;
    }

    KeychainException* clone() const override {
        return new KeychainException(*this);
    }

    const char* what() const noexcept override {
        return m_byteArray.constData();
    }

    QString message() const {
        return m_message;
    }

private:
    QString m_message;
    QByteArray m_byteArray;
};

#endif // OPA_UTILS_H
