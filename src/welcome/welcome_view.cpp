/*
    SPDX-FileCopyrightText: 2022 Jiří Wolker <woljiri@gmail.com>
    SPDX-FileCopyrightText: 2022 Eugene Popov <popov895@ukr.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "welcome_view.h"

#include "main/main_window.h"
#include "recent_item_model.h"

#include <KAboutData>
#include <KConfigGroup>
#include <KIO/OpenFileManagerWindowJob>
#include <KIconLoader>
#include <KLocalizedString>
#include <KRecentFilesAction>
#include <KSharedConfig>
#include <QClipboard>
#include <QDesktopServices>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QMenu>
#include <QTimer>

class Placeholder : public QLabel {
public:
    explicit Placeholder(QWidget* parent = nullptr) : QLabel(parent) {
        setAlignment(Qt::AlignCenter);
        setMargin(20);
        setWordWrap(true);
        // Match opacity of QML placeholder label component
        auto* opacityEffect = new QGraphicsOpacityEffect;
        opacityEffect->setOpacity(0.5);
        setGraphicsEffect(opacityEffect);
    }
};

WelcomeView::WelcomeView(MainWindow* mainWindow, QWidget* parent) : QScrollArea(parent), mainWindow_(mainWindow) {
    setupUi(this);

    listViewRecentItems->setSelectionMode(QAbstractItemView::SingleSelection);

    const KAboutData aboutData = KAboutData::applicationData();
    labelTitle->setText(i18n("Welcome to %1", aboutData.displayName()));
    labelDescription->setText(aboutData.shortDescription());
    labelIcon->setPixmap(aboutData.programLogo().value<QIcon>().pixmap(KIconLoader::SizeEnormous));

    labelRecentItems->setText(i18n("Recent Documents"));

    m_placeholderRecentItems = new Placeholder;
    m_placeholderRecentItems->setText(i18n("No recent documents"));

    auto* layoutPlaceholderRecentItems = new QVBoxLayout;
    layoutPlaceholderRecentItems->addWidget(m_placeholderRecentItems);
    listViewRecentItems->setLayout(layoutPlaceholderRecentItems);

    m_recentItemsModel = new RecentItemsModel(this);
    connect(m_recentItemsModel, &RecentItemsModel::modelReset, this, [this]() {
        const bool noRecentItems = m_recentItemsModel->rowCount() == 0;
        buttonClearRecentItems->setDisabled(noRecentItems);
        m_placeholderRecentItems->setVisible(noRecentItems);
    });

    KRecentFilesAction* recentFilesAction = mainWindow_->recentFilesAction();
    m_recentItemsModel->refresh(recentFilesAction->urls());
    recentFilesAction->menu()->installEventFilter(this);

    listViewRecentItems->setModel(m_recentItemsModel);
    connect(
        listViewRecentItems,
        &QListView::customContextMenuRequested,
        this,
        &WelcomeView::onRecentItemsContextMenuRequested
    );
    connect(listViewRecentItems, &QListView::activated, this, [this](const QModelIndex& index) {
        if (index.isValid()) {
            const QUrl url = m_recentItemsModel->url(index);
            Q_ASSERT(url.isValid());
            mainWindow_->openUrl(url);
        }
    });

    connect(buttonNewFile, &QPushButton::clicked, mainWindow_, &MainWindow::newFile);
    connect(buttonOpenFile, &QPushButton::clicked, mainWindow_, &MainWindow::openFile);
    connect(buttonClearRecentItems, &QPushButton::clicked, this, [recentFilesAction]() { recentFilesAction->clear(); });

    connect(labelHomepage, qOverload<>(&KUrlLabel::leftClickedUrl), this, [aboutData]() {
        QDesktopServices::openUrl(QUrl(aboutData.homepage()));
    });
    // TODO
    // connect(labelContribute, qOverload<>(&KUrlLabel::leftClickedUrl), this, []() {
    //     QDesktopServices::openUrl(QUrl(QStringLiteral("https://kate-editor.org/join-us")));
    // });
    // connect(labelHandbook, qOverload<>(&KUrlLabel::leftClickedUrl), this, [this]() {
    //     // TODO
    //     // m_viewManager->mainWindow()->appHelpActivated();
    // });

    const auto* showForNewWindowKey = "showWelcome";
    auto configGroup = KSharedConfig::openConfig()->group(QStringLiteral("General"));
    checkBoxShowForNewWindow->setChecked(configGroup.readEntry(showForNewWindowKey, true));
    connect(
        checkBoxShowForNewWindow,
        &QCheckBox::toggled,
        this,
        [configGroup, showForNewWindowKey](bool checked) mutable {
            configGroup.writeEntry(showForNewWindowKey, checked);
        }
    );

    // TODO
    // connect(qApp(), &QApplication::configurationChanged, this, [this, configGroup]() {
    //     checkBoxShowForNewWindow->setChecked(configGroup.readEntry(showForNewWindowKey, true));
    // });

    updateFonts();
    updateButtons();
}

bool WelcomeView::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::FontChange:
            updateFonts();
            updateButtons();
            break;
        case QEvent::Resize:
            if (updateLayout()) {
                return true;
            }
            break;
        default:
            break;
    }

    return QScrollArea::event(event);
}

void WelcomeView::resizeEvent(QResizeEvent* event) {
    QScrollArea::resizeEvent(event);

    updateLayout();
}

bool WelcomeView::eventFilter(QObject* watched, QEvent* event) {
    auto* recentFilesAction = mainWindow_->recentFilesAction();
    if (watched == recentFilesAction->menu()) {
        switch (event->type()) {
            case QEvent::ActionAdded:
            case QEvent::ActionRemoved:
                // since the KRecentFilesAction doesn't notify about adding or
                // deleting items, we should use this dirty trick to find out
                // the KRecentFilesAction has changed
                QTimer::singleShot(0, this, [this, recentFilesAction]() {
                    m_recentItemsModel->refresh(recentFilesAction->urls());
                });
                break;
            default:
                break;
        }
    }

    return QScrollArea::eventFilter(watched, event);
}

void WelcomeView::onRecentItemsContextMenuRequested(const QPoint& pos) {
    const QModelIndex index = listViewRecentItems->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const QUrl url = m_recentItemsModel->url(index);
    Q_ASSERT(url.isValid());

    QMenu contextMenu(listViewRecentItems);

    const auto selectedIndexes = listViewRecentItems->selectionModel()->selectedIndexes();

    auto* action = new QAction(i18n("Copy &Location"), this);
    action->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy-path")));
    connect(action, &QAction::triggered, this, [url]() {
        qApp->clipboard()->setText(url.toString(QUrl::PreferLocalFile));
    });
    contextMenu.addAction(action);

    action = new QAction(i18n("&Open Containing Folder"), this);
    action->setEnabled(url.isLocalFile());
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-open-folder")));
    connect(action, &QAction::triggered, this, [url]() { KIO::highlightInFileManager({url}); });
    contextMenu.addAction(action);

    action = new QAction(i18n("&Remove"), this);
    action->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    connect(action, &QAction::triggered, this, [this, selectedIndexes]() {
        KRecentFilesAction* recentFilesAction = mainWindow_->recentFilesAction();
        for (const auto& selectedIndex: selectedIndexes) {
            if (const auto url_ = m_recentItemsModel->url(selectedIndex); url_.isValid()) {
                recentFilesAction->removeUrl(url_);
            }
        }
    });
    contextMenu.addAction(action);

    contextMenu.exec(listViewRecentItems->mapToGlobal(pos));
}

void WelcomeView::updateButtons() const {
    const QList buttons{buttonNewFile, buttonOpenFile};
    const int maxWidth =
        std::accumulate(buttons.cbegin(), buttons.cend(), 0, [](int maxWidth_, const QPushButton* button) {
            return std::max(maxWidth_, button->sizeHint().width());
        });
    for (auto* button: buttons) {
        button->setFixedWidth(maxWidth);
    }
}

void WelcomeView::updateFonts() const {
    QFont titleFont = font();
    titleFont.setPointSize(titleFont.pointSize() + 6);
    titleFont.setWeight(QFont::Bold);
    labelTitle->setFont(titleFont);

    QFont panelTitleFont = font();
    panelTitleFont.setPointSize(panelTitleFont.pointSize() + 2);
    labelRecentItems->setFont(panelTitleFont);
    labelHelp->setFont(panelTitleFont);

    QFont placeholderFont = font();
    placeholderFont.setPointSize(qRound(placeholderFont.pointSize() * 1.3));
    m_placeholderRecentItems->setFont(placeholderFont);
    if (m_placeholderSavedSessions) {
        m_placeholderSavedSessions->setFont(placeholderFont);
    }
}

bool WelcomeView::updateLayout() {
    // Align labelHelp with labelRecentItems
    labelHelp->setMinimumHeight(labelRecentItems->height());

    bool result = false;

    // show/hide widgetHeader depending on the view height
    if (widgetHeader->isVisible()) {
        if (height() <= frameContent->height()) {
            widgetHeader->hide();
            result = true;
        }
    } else {
        const int implicitHeight = frameContent->height() + widgetHeader->height() + layoutContent->spacing();
        if (height() > implicitHeight) {
            widgetHeader->show();
            result = true;
        }
    }

    // show/hide widgetHelp depending on the view height
    if (widgetHelp->isVisible()) {
        if (width() <= frameContent->width()) {
            widgetHelp->hide();
            result = true;
        }
    } else {
        const int implicitWidth = frameContent->width() + widgetHelp->width() + layoutPanels->horizontalSpacing();
        if (width() > implicitWidth) {
            widgetHelp->show();
            return true;
        }
    }

    return result;
}
