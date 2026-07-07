/*
 * SPDX-FileCopyrightText: Niko Strijbol <niko@strijbol.be>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/**
 * @file
 *
 * This file parses Gramps XML and inserts it into the Opa database.
 * The parser / importer current only support version 1.7.x Gramps XML version.
 * Newer versions might be supported if they do not break compatibility.
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
#include <QThread>

using namespace Qt::StringLiterals;

static const auto GRAMPS_ID = QLatin1String("gramps_id");

static void relaxNgErrorCollector(void* ctx, const char* msg, ...) {
    if (!ctx) {
        return;
    }

    auto* errorList = static_cast<QStringList*>(ctx);

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

static QString attrStr(const xmlNode* node, const char* name) {
    // ReSharper disable once CppCStyleCast
    xmlChar* val = xmlGetProp(node, BAD_CAST name);
    if (!val) {
        return {};
    }

    QString result = QString::fromUtf8(val);
    xmlFree(val);
    return result;
}

static QString textContent(const xmlNode* node) {
    xmlChar* val = xmlNodeGetContent(node);
    if (!val) {
        return {};
    }
    QString result = QString::fromUtf8(val);
    xmlFree(val);
    return result;
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
        GrampsXmlAnalysis result {
            .valid = false,
            .error = i18n("Could not parse XML file"),
        };
        promise.addResult(std::move(result));
        return;
    }
    GrampsXmlRoot document(rawDocument, xmlFreeDoc);

    int validationResult = xmlRelaxNGValidateDoc(validationContext.get(), document.get());

    if (validationResult > 0) {
        qDebug() << "Invalid Gramps XML file.";

        GrampsXmlAnalysis result {
            .valid = false,
            .error = validationErrors.join(u"\n"_s),
        };
        promise.addResult(std::move(result));
        return;
    } else if (validationResult < 0) {
        qWarning() << "Internal error while validating XML file.";
        promise.setException(ResourceNotFoundException());
        return;
    }

    assert(validationResult == 0);

    GrampsXmlAnalysis result {
        .document = std::move(document),
        .valid = true,
        .error = {},
    };

    // <database>
    const xmlNode* root = xmlDocGetRootElement(result.document.get());

    for (const xmlNode* c = root->children; c; c = c->next) {
        if (nodeNameIs(c, "people")) {
            result.people = countElementChildren(c);
        } else if (nodeNameIs(c, "families")) {
            result.families = countElementChildren(c);
        } else if (nodeNameIs(c, "events")) {
            result.events = countElementChildren(c);
        } else if (nodeNameIs(c, "sources")) {
            result.sources = countElementChildren(c);
        } else if (nodeNameIs(c, "places")) {
            result.places = countElementChildren(c);
        } else if (nodeNameIs(c, "media")) {
            result.media = countElementChildren(c);
        } else if (nodeNameIs(c, "repositories")) {
            result.repositories = countElementChildren(c);
        } else if (nodeNameIs(c, "notes")) {
            result.notes = countElementChildren(c);
        } else if (nodeNameIs(c, "citations")) {
            result.citations = countElementChildren(c);
        }
    }

    promise.addResult(std::move(result));
}

struct GrampsDate {
    QString type;
    // dateval.val or datestr.val
    QString value;
    // daterange/datespan.start
    QString start;
    // daterange/datespan.stop
    QString stop;
    // before/after/about (dateval only)
    QString modifier;
    // estimated/calculated
    QString quality;
    QString calendarFormat;
    bool isDualDated = false;
    QString newYear;
};

struct GrampsSurname {
    QString surname;
    QString prefix;
    QString derivation;
    QString connector;
    bool isPrimary = false;
};

struct GrampsAddress {
    GrampsDate date;
    QString street;
    QString locality;
    QString city;
    QString county;
    QString state;
    QString country;
    QString postal;
    QString phone;
    QList<QString> noteHandles;
    QList<QString> citationHandles;
};

struct GrampsName {
    bool isAlternate = false;
    QString type;
    QString sortAs;
    QString displayAs;
    QString givenNames;
    QString callName;
    QList<GrampsSurname> surnames;
    QString suffix;
    QString title;
    QString nickname;
    QString familyNickname;
    QString groupAs;
    GrampsDate date;
    QList<QString> noteHandles;
    QList<QString> citationHandles;
};

struct GrampsEventRef {
    QString eventHandle;
    QString role;
    QList<QString> noteHandles;
    QList<QString> citationHandles;
};

struct GrampsMediaRef {
    QString mediaHandle;
    QList<QString> citationHandles;
    QList<QString> noteHandles;
};

struct GrampsPersonRef {
    QString personHandle;
    QString relation;
    QList<QString> citationHandles;
    QList<QString> noteHandles;
};

struct GrampsChildRef {
    QString personHandle;
    QString motherRelationType;
    QString fatherRelationType;
    QList<QString> citationHandles;
    QList<QString> noteHandles;
};

struct GrampsPerson {
    QString handle, id, sex;
    QList<GrampsName> names;
    QList<GrampsEventRef> eventRefs;
    QList<GrampsMediaRef> mediaRefs;
    QList<GrampsAddress> addresses;
    QList<GrampsPersonRef> personRefs;
    QList<QString> noteHandles;
    QList<QString> citationHandles;
};

struct GrampsFamily {
    QString handle, id;
    QString relationshipType;
    QString fatherHandle;
    QString motherHandle;
    QList<GrampsEventRef> eventRefs;
    QList<GrampsChildRef> childRefs;
    QList<GrampsMediaRef> mediaRefs;
    GrampsDate date;
    QList<QString> noteHandles;
    QList<QString> citationHandles;
};

struct GrampsEvent {
    QString handle, id, type, description;
    GrampsDate date;
    QString placeHandle;
    QList<GrampsMediaRef> mediaRefs;
    QList<QString> noteHandles;
    QList<QString> citationHandles;
};

struct GrampsPlaceName {
    QString value;
    QString language;
    GrampsDate date;
};

struct GrampsPlace {
    QString handle, id, type;
    QString title;
    QString code;
    QList<GrampsPlaceName> names;
    double latitude = 0.0, longitude = 0.0;
    bool hasCoordinates = false;
    QList<QString> parentHandles;
    QList<GrampsMediaRef> mediaRefs;
    QList<QString> noteHandles;
    QList<QString> citationHandles;
};

struct GrampsNote {
    QString handle, id, type;
    QString text;
};

struct GrampsSrcAttribute {
    QString type;
    QString value;
};

struct GrampsRepositoryRef {
    QString repositoryHandle;
    QString callNumber;
    QString medium;
    QList<QString> noteHandles;
};

struct GrampsCitation {
    QString handle, id;
    GrampsDate date;
    QString page;
    QString confidence;
    QString sourceHandle;
    QList<GrampsSrcAttribute> sourceAttributes;
    QList<GrampsMediaRef> mediaRefs;
    QList<QString> noteHandles;
};

struct GrampsSource {
    QString handle, id;
    QString title;
    QString author;
    QString publicationInfo;
    QString abbreviation;
    QList<GrampsSrcAttribute> sourceAttributes;
    QList<GrampsRepositoryRef> repositoryRefs;
    QList<GrampsMediaRef> mediaRefs;
    QList<QString> noteHandles;
};

struct GrampsMedia {
    QString handle, id;
    QString filePath;
    QString mimeType;
    QString checksum;
    QString description;
    GrampsDate date;
    QList<QString> noteHandles;
    QList<QString> citationHandles;
};

struct GrampsRepository {
    QString handle, id;
    QString name;
    QString type;
    QList<GrampsAddress> addresses;
    QList<QString> noteHandles;
};

struct GrampsData {
    QHash<QString, GrampsPerson> people;
    QHash<QString, GrampsFamily> families;
    QHash<QString, GrampsEvent> events;
    QHash<QString, GrampsPlace> places;
    QHash<QString, GrampsNote> notes;
    QHash<QString, GrampsCitation> citations;
    QHash<QString, GrampsSource> sources;
    QHash<QString, GrampsMedia> media;
    QHash<QString, GrampsRepository> repositories;
};

static GrampsDate parseDate(const xmlNode* node) {
    GrampsDate date;

    if (nodeNameIs(node, "daterange")) {
        date = {
            .type = u"daterange"_s,
            .start = attrStr(node, "start"),
            .stop = attrStr(node, "stop"),
            .quality = attrStr(node, "quality"),
            .calendarFormat = attrStr(node, "cformat"),
            .isDualDated = attrStr(node, "dualdated") == u"1"_s,
            .newYear = attrStr(node, "newyear"),
        };
    } else if (nodeNameIs(node, "datespan")) {
        date = {
            .type = u"datespan"_s,
            .start = attrStr(node, "start"),
            .stop = attrStr(node, "stop"),
            .quality = attrStr(node, "quality"),
            .calendarFormat = attrStr(node, "cformat"),
            .isDualDated = attrStr(node, "dualdated") == u"1"_s,
            .newYear = attrStr(node, "newyear"),
        };
    } else if (nodeNameIs(node, "dateval")) {
        date = {
            .type = u"dateval"_s,
            .value = attrStr(node, "val"),
            .modifier = attrStr(node, "type"),
            .quality = attrStr(node, "quality"),
            .calendarFormat = attrStr(node, "cformat"),
            .isDualDated = attrStr(node, "dualdated") == u"1"_s,
            .newYear = attrStr(node, "newyear"),
        };
    } else if (nodeNameIs(node, "datestr")) {
        date = {
            .type = u"datestr"_s,
            .value = attrStr(node, "val"),
        };
    } else {
        qWarning() << "Unknown node type" << node->type;
    }

    return date;
}

static bool isDateNode(const xmlNode* c) {
    return nodeNameIs(c, "dateval") || nodeNameIs(c, "daterange") || nodeNameIs(c, "datespan") ||
           nodeNameIs(c, "datestr");
}

static GrampsSurname parseSurname(const xmlNode* node) {
    return {
        .surname = textContent(node),
        .prefix = attrStr(node, "prefix"),
        .derivation = attrStr(node, "derivation"),
        .connector = attrStr(node, "connector"),
        .isPrimary = attrStr(node, "prim") == u"1"_s,
    };
}

static GrampsName parseName(const xmlNode* node) {
    GrampsName name {
        .isAlternate = attrStr(node, "alt") == u"1"_s,
        .type = attrStr(node, "type"),
        .sortAs = attrStr(node, "sort"),
        .displayAs = attrStr(node, "display"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "first")) {
            name.givenNames = textContent(c);
        } else if (nodeNameIs(c, "call")) {
            name.callName = textContent(c);
        } else if (nodeNameIs(c, "surname")) {
            name.surnames.append(parseSurname(c));
        } else if (nodeNameIs(c, "suffix")) {
            name.suffix = textContent(c);
        } else if (nodeNameIs(c, "title")) {
            name.title = textContent(c);
        } else if (nodeNameIs(c, "nick")) {
            name.nickname = textContent(c);
        } else if (nodeNameIs(c, "familynick")) {
            name.familyNickname = textContent(c);
        } else if (nodeNameIs(c, "group")) {
            name.groupAs = textContent(c);
        } else if (nodeNameIs(c, "noteref")) {
            name.noteHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "citationref")) {
            name.citationHandles.append(attrStr(c, "hlink"));
        } else if (isDateNode(c)) {
            name.date = parseDate(c);
        }
    }

    return name;
}

static GrampsEventRef parseEventRef(const xmlNode* node) {
    GrampsEventRef ref {
        .eventHandle = attrStr(node, "hlink"),
        .role = attrStr(node, "role"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "noteref")) {
            ref.noteHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "citationref")) {
            ref.citationHandles.append(attrStr(c, "hlink"));
        }
    }

    return ref;
}

static GrampsMediaRef parseMediaRef(const xmlNode* node) {
    GrampsMediaRef ref {
        .mediaHandle = attrStr(node, "hlink"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "citationref")) {
            ref.citationHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "noteref")) {
            ref.noteHandles.append(attrStr(c, "hlink"));
        }
    }

    return ref;
}

static GrampsPersonRef parsePersonRef(const xmlNode* node) {
    GrampsPersonRef ref {
        .personHandle = attrStr(node, "hlink"),
        .relation = attrStr(node, "rel"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "citationref")) {
            ref.citationHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "noteref")) {
            ref.noteHandles.append(attrStr(c, "hlink"));
        }
    }

    return ref;
}

static GrampsChildRef parseChildRef(const xmlNode* node) {
    GrampsChildRef ref {
        .personHandle = attrStr(node, "hlink"),
        .motherRelationType = attrStr(node, "mrel"),
        .fatherRelationType = attrStr(node, "frel"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "citationref")) {
            ref.citationHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "noteref")) {
            ref.noteHandles.append(attrStr(c, "hlink"));
        }
    }

    return ref;
}

static GrampsSrcAttribute parseSrcAttribute(const xmlNode* node) {
    return {
        .type = attrStr(node, "type"),
        .value = attrStr(node, "value"),
    };
}

static GrampsRepositoryRef parseRepositoryRef(const xmlNode* node) {
    GrampsRepositoryRef ref {
        .repositoryHandle = attrStr(node, "hlink"),
        .callNumber = attrStr(node, "callno"),
        .medium = attrStr(node, "medium"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "noteref")) {
            ref.noteHandles.append(attrStr(c, "hlink"));
        }
    }

    return ref;
}

static GrampsPerson parsePerson(const xmlNode* node) {
    GrampsPerson person {
        .handle = attrStr(node, "handle"),
        .id = attrStr(node, "id"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "gender")) {
            person.sex = textContent(c);
        } else if (nodeNameIs(c, "name")) {
            person.names.append(parseName(c));
        } else if (nodeNameIs(c, "eventref")) {
            person.eventRefs.append(parseEventRef(c));
        } else if (nodeNameIs(c, "objref")) {
            person.mediaRefs.append(parseMediaRef(c));
        } else if (nodeNameIs(c, "personref")) {
            person.personRefs.append(parsePersonRef(c));
        } else if (nodeNameIs(c, "noteref")) {
            person.noteHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "citationref")) {
            person.citationHandles.append(attrStr(c, "hlink"));
        }
    }

    return person;
}

static GrampsFamily parseFamily(const xmlNode* node) {
    GrampsFamily family {
        .handle = attrStr(node, "handle"),
        .id = attrStr(node, "id"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "rel")) {
            family.relationshipType = attrStr(c, "type");
        } else if (nodeNameIs(c, "father")) {
            family.fatherHandle = attrStr(c, "hlink");
        } else if (nodeNameIs(c, "mother")) {
            family.motherHandle = attrStr(c, "hlink");
        } else if (nodeNameIs(c, "eventref")) {
            family.eventRefs.append(parseEventRef(c));
        } else if (nodeNameIs(c, "childref")) {
            family.childRefs.append(parseChildRef(c));
        } else if (nodeNameIs(c, "objref")) {
            family.mediaRefs.append(parseMediaRef(c));
        } else if (nodeNameIs(c, "noteref")) {
            family.noteHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "citationref")) {
            family.citationHandles.append(attrStr(c, "hlink"));
        } else if (isDateNode(c)) {
            family.date = parseDate(c);
        }
    }

    return family;
}

static GrampsEvent parseEvent(const xmlNode* node) {
    GrampsEvent event {
        .handle = attrStr(node, "handle"),
        .id = attrStr(node, "id"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "type")) {
            event.type = textContent(c);
        } else if (nodeNameIs(c, "place")) {
            event.placeHandle = attrStr(c, "hlink");
        } else if (nodeNameIs(c, "description")) {
            event.description = textContent(c);
        } else if (nodeNameIs(c, "objref")) {
            event.mediaRefs.append(parseMediaRef(c));
        } else if (nodeNameIs(c, "noteref")) {
            event.noteHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "citationref")) {
            event.citationHandles.append(attrStr(c, "hlink"));
        } else if (isDateNode(c)) {
            event.date = parseDate(c);
        }
    }

    return event;
}

static GrampsPlace parsePlace(const xmlNode* node) {
    GrampsPlace place {
        .handle = attrStr(node, "handle"),
        .id = attrStr(node, "id"),
        .type = attrStr(node, "type"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "ptitle")) {
            place.title = textContent(c);
        } else if (nodeNameIs(c, "code")) {
            place.code = textContent(c);
        } else if (nodeNameIs(c, "pname")) {
            GrampsPlaceName name {
                .value = attrStr(c, "value"),
                .language = attrStr(c, "lang"),
            };
            for (const xmlNode* d = c->children; d; d = d->next) {
                if (isDateNode(d)) {
                    name.date = parseDate(d);
                }
            }
            place.names.append(name);
        } else if (nodeNameIs(c, "coord")) {
            place.latitude = attrStr(c, "lat").toDouble();
            place.longitude = attrStr(c, "long").toDouble();
            place.hasCoordinates = true;
        } else if (nodeNameIs(c, "placeref")) {
            place.parentHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "location")) {
            // TODO
        } else if (nodeNameIs(c, "objref")) {
            place.mediaRefs.append(parseMediaRef(c));
        } else if (nodeNameIs(c, "noteref")) {
            place.noteHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "citationref")) {
            place.citationHandles.append(attrStr(c, "hlink"));
        }
    }

    return place;
}

static GrampsNote parseNote(const xmlNode* node) {
    GrampsNote note {
        .handle = attrStr(node, "handle"),
        .id = attrStr(node, "id"),
        .type = attrStr(node, "type"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "text")) {
            note.text = textContent(c);
        }
    }

    return note;
}

static GrampsCitation parseCitation(const xmlNode* node) {
    GrampsCitation citation {
        .handle = attrStr(node, "handle"),
        .id = attrStr(node, "id"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "page")) {
            citation.page = textContent(c);
        } else if (nodeNameIs(c, "confidence")) {
            citation.confidence = textContent(c);
        } else if (nodeNameIs(c, "sourceref")) {
            citation.sourceHandle = attrStr(c, "hlink");
        } else if (nodeNameIs(c, "objref")) {
            citation.mediaRefs.append(parseMediaRef(c));
        } else if (nodeNameIs(c, "srcattribute")) {
            citation.sourceAttributes.append(parseSrcAttribute(c));
        } else if (nodeNameIs(c, "sourceref")) {
            citation.sourceHandle = attrStr(c, "hlink");
        } else if (isDateNode(c)) {
            citation.date = parseDate(c);
        }
    }

    return citation;
}

static GrampsSource parseSource(const xmlNode* node) {
    GrampsSource source {
        .handle = attrStr(node, "handle"),
        .id = attrStr(node, "id"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "stitle")) {
            source.title = textContent(c);
        } else if (nodeNameIs(c, "sauthor")) {
            source.author = textContent(c);
        } else if (nodeNameIs(c, "spubinfo")) {
            source.publicationInfo = textContent(c);
        } else if (nodeNameIs(c, "sabbrev")) {
            source.abbreviation = textContent(c);
        } else if (nodeNameIs(c, "noteref")) {
            source.noteHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "objref")) {
            source.mediaRefs.append(parseMediaRef(c));
        } else if (nodeNameIs(c, "srcattribute")) {
            source.sourceAttributes.append(parseSrcAttribute(c));
        } else if (nodeNameIs(c, "reporef")) {
            source.repositoryRefs.append(parseRepositoryRef(c));
        }
    }

    return source;
}

// Called object-content in the XML
static GrampsMedia parseMedia(const xmlNode* node) {
    GrampsMedia media {
        .handle = attrStr(node, "handle"),
        .id = attrStr(node, "id"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "file")) {
            media.filePath = attrStr(c, "src");
            media.mimeType = attrStr(c, "mime");
            media.checksum = attrStr(c, "checksum");
            media.description = attrStr(c, "description");
        } else if (nodeNameIs(c, "noteref")) {
            media.noteHandles.append(attrStr(c, "hlink"));
        } else if (nodeNameIs(c, "citationref")) {
            media.citationHandles.append(attrStr(c, "hlink"));
        } else if (isDateNode(c)) {
            media.date = parseDate(c);
        }
    }

    return media;
}

static GrampsRepository parseRepository(const xmlNode* node) {
    GrampsRepository repository {
        .handle = attrStr(node, "handle"),
        .id = attrStr(node, "id"),
    };

    for (const xmlNode* c = node->children; c; c = c->next) {
        if (nodeNameIs(c, "rname")) {
            repository.name = textContent(c);
        } else if (nodeNameIs(c, "type")) {
            repository.type = textContent(c);
        } else if (nodeNameIs(c, "noteref")) {
            repository.noteHandles.append(attrStr(c, "hlink"));
        }
    }

    return repository;
}

static GrampsData parseGrampsData(QPromise<bool>& promise, const GrampsXmlRoot& root) {
    int progress = 1;

    const xmlNode* db = xmlDocGetRootElement(root.get());
    if (!db) {
        return {};
    }

    GrampsData data;

    for (const xmlNode* c = db->children; c; c = c->next) {
        // TODO: remove this maybe?
        QThread::msleep(10);
        if (nodeNameIs(c, "people")) {
            for (const xmlNode* d = c->children; d; d = d->next) {
                if (d->type == XML_ELEMENT_NODE) {
                    promise.setProgressValueAndText(progress++, i18n("Parsing people"));
                    auto person = parsePerson(d);
                    data.people.insert(person.id, person);
                }
            }
        } else if (nodeNameIs(c, "families")) {
            for (const xmlNode* d = c->children; d; d = d->next) {
                if (d->type == XML_ELEMENT_NODE) {
                    promise.setProgressValueAndText(progress++, i18n("Parsing families"));
                    auto family = parseFamily(d);
                    data.families.insert(family.id, family);
                }
            }
        } else if (nodeNameIs(c, "events")) {
            for (const xmlNode* d = c->children; d; d = d->next) {
                if (d->type == XML_ELEMENT_NODE) {
                    promise.setProgressValueAndText(progress++, i18n("Parsing events"));
                    auto event = parseEvent(d);
                    data.events.insert(event.id, event);
                }
            }
        } else if (nodeNameIs(c, "sources")) {
            for (const xmlNode* d = c->children; d; d = d->next) {
                if (d->type == XML_ELEMENT_NODE) {
                    promise.setProgressValueAndText(progress++, i18n("Parsing sources"));
                    auto source = parseSource(d);
                    data.sources.insert(source.id, source);
                }
            }
        } else if (nodeNameIs(c, "places")) {
            for (const xmlNode* d = c->children; d; d = d->next) {
                if (d->type == XML_ELEMENT_NODE) {
                    promise.setProgressValueAndText(progress++, i18n("Parsing places"));
                    auto place = parsePlace(d);
                    data.places.insert(place.id, place);
                }
            }
        } else if (nodeNameIs(c, "citations")) {
            for (const xmlNode* d = c->children; d; d = d->next) {
                if (d->type == XML_ELEMENT_NODE) {
                    promise.setProgressValueAndText(progress++, i18n("Parsing citations"));
                    auto citation = parseCitation(d);
                    data.citations.insert(citation.id, citation);
                }
            }
        } else if (nodeNameIs(c, "objects")) {
            for (const xmlNode* d = c->children; d; d = d->next) {
                if (d->type == XML_ELEMENT_NODE) {
                    promise.setProgressValueAndText(progress++, i18n("Parsing media"));
                    auto media = parseMedia(d);
                    data.media.insert(media.id, media);
                }
            }
        } else if (nodeNameIs(c, "repositories")) {
            for (const xmlNode* d = c->children; d; d = d->next) {
                if (d->type == XML_ELEMENT_NODE) {
                    promise.setProgressValueAndText(progress++, i18n("Parsing repositories"));
                    auto repository = parseRepository(d);
                    data.repositories.insert(repository.id, repository);
                }
            }
        } else if (nodeNameIs(c, "notes")) {
            for (const xmlNode* d = c->children; d; d = d->next) {
                if (d->type == XML_ELEMENT_NODE) {
                    promise.setProgressValueAndText(progress++, i18n("Parsing notes"));
                    auto note = parseNote(d);
                    data.notes.insert(note.id, note);
                }
            }
        }
    }

    return data;
}

void importGrampsResult(QPromise<bool>& promise, const GrampsXmlAnalysis& result, const QString& databaseName) {
    int total = result.people + result.families + result.events + result.sources + result.places + result.media +
                result.repositories + result.notes + result.citations;
    // We first parse everything into one, and then insert it, and one extra step for preparation.
    promise.setProgressRange(0, total * 2 + 1);
    promise.setProgressValueAndText(1, i18n("Preparing"));

    // Begin by creating lookup tables.
    auto data = parseGrampsData(promise, result.document);
}
