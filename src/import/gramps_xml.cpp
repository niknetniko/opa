/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "gramps_xml.h"

#include "utils/resource_exception.h"
#include <libxml/parser.h>
#include <libxml/relaxng.h>

#include <KLocalizedString>
#include <QFile>
#include <QLoggingCategory>
#include <QString>
#include <QStringList>
#include <cstdarg>


using namespace Qt::StringLiterals;


static void relaxNgErrorCollector(void* ctx, const char* msg, ...) {
    if (!ctx) {
        return;
    }

    auto *errorList = static_cast<QStringList*>(ctx);

    va_list args;
    va_start(args, msg);
    QString formattedError = QString::vasprintf(msg, args);
    va_end(args);

    errorList->append(formattedError.trimmed());
}

static int countElementChildren(const xmlNode* parent) {
    int count = 0;
    for (const xmlNode* child = parent->children; child != nullptr; child = child->next) {
        if (child->type == XML_ELEMENT_NODE) {
            ++count;
        }
    }
    return count;
}

static bool nodeNameIs(const xmlNode* node, const char* name) {
    // ReSharper disable once CppCStyleCast
    return xmlStrEqual(node->name, BAD_CAST name) != 0;
}

void validateGrampsXml(QPromise<GrampsXmlAnalysis>& promise, const QString& filename) {
    QFile rngSchemaFile(u":/schema/grampsxml-1.7.2.rng"_s);

    if (!rngSchemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open Gramps XML schema file.";
        promise.setException(ResourceNotFoundException());
        return;
    }

    QByteArray rngSchemaBytes = rngSchemaFile.readAll();
    rngSchemaFile.close();

    xmlRelaxNGParserCtxtPtr rawParserContext =
        xmlRelaxNGNewMemParserCtxt(rngSchemaBytes.constData(), rngSchemaBytes.size());
    if (!rawParserContext) {
        qWarning() << "Failed to create RelaxNG parser context from memory.";
        promise.setException(ResourceNotFoundException());
        return;
    }
    std::unique_ptr<xmlRelaxNGParserCtxt, decltype(&xmlRelaxNGFreeParserCtxt)> parserContext(
        rawParserContext,
        xmlRelaxNGFreeParserCtxt
    );

    xmlRelaxNGPtr rawSchema = xmlRelaxNGParse(parserContext.get());
    if (!rawSchema) {
        qWarning() << "Failed to create RelaxNG schema.";
        promise.setException(ResourceNotFoundException());
        return;
    }
    std::unique_ptr<xmlRelaxNG, decltype(&xmlRelaxNGFree)> schema(rawSchema, xmlRelaxNGFree);

    xmlRelaxNGValidCtxtPtr rawValidationContext = xmlRelaxNGNewValidCtxt(schema.get());
    if (!rawValidationContext) {
        qWarning() << "Failed to create RelaxNG validation context.";
        promise.setException(ResourceNotFoundException());
        return;
    }
    std::unique_ptr<xmlRelaxNGValidCtxt, decltype(&xmlRelaxNGFreeValidCtxt)> validationContext(
        rawValidationContext,
        xmlRelaxNGFreeValidCtxt
    );

    QStringList validationErrors;
    xmlRelaxNGSetValidErrors(validationContext.get(), relaxNgErrorCollector, nullptr, &validationErrors);

    QByteArray rawFilename = filename.toUtf8();
    xmlDocPtr rawDocument = xmlReadFile(rawFilename.constData(), nullptr, XML_PARSE_NONET);
    if (!rawDocument) {
        qWarning() << "Failed to parse XML file.";
        GrampsXmlAnalysis result{
            .valid = false,
            .error = i18n("Could not parse XML file"),
        };
        promise.addResult(result);
        return;
    }
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> document(rawDocument, xmlFreeDoc);

    int validationResult = xmlRelaxNGValidateDoc(validationContext.get(), document.get());

    if (validationResult > 0) {
        qDebug() << "Invalid Gramps XML file.";

        GrampsXmlAnalysis result{
            .valid = false,
            .error = validationErrors.join(u"\n"_s),
        };
        promise.addResult(result);
        return;
    } else if (validationResult < 0) {
        qWarning() << "Internal error while validating XML file.";
        promise.setException(ResourceNotFoundException());
        return;
    }

    assert(validationResult == 0);

    GrampsXmlAnalysis result{
        .valid = true,
    };

    // <database>
    if (const xmlNode* root = xmlDocGetRootElement(document.get())) {
        for (const xmlNode* container = root->children; container != nullptr; container = container->next) {
            if (container->type != XML_ELEMENT_NODE) {
                // skip whitespace/text nodes
                continue;
            }
            if (nodeNameIs(container, "people")) {
                result.people = countElementChildren(container);
            } else if (nodeNameIs(container, "families")) {
                result.families = countElementChildren(container);
            } else if (nodeNameIs(container, "events")) {
                result.events = countElementChildren(container);
            } else if (nodeNameIs(container, "sources")) {
                result.sources = countElementChildren(container);
            } else if (nodeNameIs(container, "places")) {
                result.places = countElementChildren(container);
            }
        }
    }

    promise.addResult(result);
}
