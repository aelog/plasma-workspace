/*
 *   Copyright 2008 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "recentdocuments.h"

#include <QAction>
#include <QDir>
#include <QMimeData>

#include <KDesktopFile>
#include <KConfigGroup>
#include <KDirWatch>
#include <KRun>
#include <KRecentDocument>
#include <KLocalizedString>
#include <KIO/OpenFileManagerWindowJob>

K_EXPORT_PLASMA_RUNNER(recentdocuments, RecentDocuments)

RecentDocuments::RecentDocuments(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    setObjectName( QStringLiteral("Recent Documents" ));
    loadRecentDocuments();
    // listen for changes to the list of recent documents
    KDirWatch *recentDocWatch = new KDirWatch(this);
    recentDocWatch->addDir(KRecentDocument::recentDocumentDirectory(), KDirWatch::WatchFiles);
    connect(recentDocWatch, &KDirWatch::created, this, &RecentDocuments::loadRecentDocuments);
    connect(recentDocWatch, &KDirWatch::deleted, this, &RecentDocuments::loadRecentDocuments);
    connect(recentDocWatch, &KDirWatch::dirty, this, &RecentDocuments::loadRecentDocuments);
    addSyntax(Plasma::RunnerSyntax(QStringLiteral(":q:"), i18n("Looks for documents recently used with names matching :q:.")));
}

RecentDocuments::~RecentDocuments()
{
}

void RecentDocuments::loadRecentDocuments()
{
    m_recentdocuments = KRecentDocument::recentDocuments();
}


void RecentDocuments::match(Plasma::RunnerContext &context)
{
    if (m_recentdocuments.isEmpty()) {
        return;
    }

    const QString term = context.query();
    if (term.length() < 3) {
        return;
    }

    const QString homePath = QDir::homePath();

    foreach (const QString &document, m_recentdocuments) {
        if (!context.isValid()) {
            return;
        }

        if (document.contains(term, Qt::CaseInsensitive)) {
            KDesktopFile config(document);
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::PossibleMatch);
            match.setRelevance(1.0);
            match.setIconName(config.readIcon());
            match.setData(config.readUrl());
            match.setText(config.readName());

            QUrl folderUrl = QUrl(config.readUrl()).adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash);
            if (folderUrl.isLocalFile()) {
                QString folderPath = folderUrl.toLocalFile();
                if (folderPath.startsWith(homePath)) {
                    folderPath.replace(0, homePath.length(), QStringLiteral("~"));
                }
                match.setSubtext(folderPath);
            } else {
                match.setSubtext(folderUrl.toDisplayString());
            }

            context.addMatch(match);
        }
    }
}

void RecentDocuments::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    const QString url = match.data().toString();

    if (match.selectedAction() && match.selectedAction()->data().toString() == QLatin1String("openParentDir")) {
        KIO::highlightInFileManager({QUrl(url)});
        return;
    }

    new KRun(QUrl(url), 0);
}

QList<QAction *> RecentDocuments::actionsForMatch(const Plasma::QueryMatch &match)
{
    Q_UNUSED(match)

    const QString openParentDirId = QStringLiteral("openParentDir");

    if (!action(openParentDirId)) {
        (addAction(openParentDirId, QIcon::fromTheme(QStringLiteral("document-open-folder")), i18n("Open Containing Folder")))->setData(openParentDirId);
    }

    QList<QAction *> actions;

    if (QUrl(match.data().toString()).isLocalFile()) {
        actions << action(openParentDirId);
    }

    return actions;
}

QMimeData * RecentDocuments::mimeDataForMatch(const Plasma::QueryMatch& match)
{
    QMimeData *result = new QMimeData();
    result->setText(match.data().toString());
    return result;
}

#include "recentdocuments.moc"
