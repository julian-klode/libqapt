/***************************************************************************
 *   Copyright © 2010 Jonathan Thomas <echidnaman@kubuntu.org>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef QAPT_BACKEND_H
#define QAPT_BACKEND_H

#include <QtCore/QSet>
#include <QtCore/QVariantList>

#include "globals.h"
#include "group.h"
#include "package.h"

class pkgSourceList;

/**
 * The QApt namespace is the main namespace for LibQApt. All classes in this
 * library fall under this namespace.
 */
namespace QApt {

class BackendPrivate;

/**
 * @brief The main entry point for performing operations with the dpkg database
 *
 * Backend encapsulates all the needed logic to perform most apt operations.
 * It implements the initializing of the database and all requests to/for the
 * database. Please note that you \b _MUST_ call init() before doing any
 * further operations to the backend, or else risk encountering undefined
 * behavior.
 *
 * @author Jonathan Thomas
 */
class Backend : public QObject
{
    Q_OBJECT
public:
     /**
      * Default constructor
      */
    explicit Backend();

     /**
      * Default destructor
      */
    virtual ~Backend();

    /**
     * Initializes the Apt database for usage. It sets up everything the backend
     * will need to perform all operations. Please note that you @b _MUST_ call
     * this function before doing any further operations in the backend, or else
     * risk encountering undefined behavior.
     *
     * @return @c true if initialization was successful
     * @return @c false if there was a problem initializing
     */
    bool init();

    /**
     * Repopulates the internal package cache, package list, and group list.
     * Mostly used internally, like after an update or a package installation
     * or removal.
     *
     */
    void reloadCache();

    /**
     * Queries the backend for a Package object for the specified name.
     * @b _WARNING_ :
     * Note that at the moment this method is unsafe to use unless you are sure
     * that a package with the name you specified, as the library currently
     * does not have a null package to return in the case where a package
     * with the specified name doesn't exists, and returns nothing, resulting
     * in a crash. For the moment, you'll want to check your Package objects to
     * see if they are null before trying to access them.
     *
     * @param name name used to specify the package returned
     *
     * @return A pointer to a @c Package defined by the specified name
     */
    Package *package(const QString &name) const;

    /**
     * Queries the backend for the total number of packages in the Apt
     * database, discarding no-longer-existing packages that linger on in the
     * status cache (That have a version of 0)
     *
     * @return The total number of packages in the Apt database
     */
    int packageCount();

    /**

     * Queries the backend for the total number of packages in the Apt
     * database, discarding no-longer-existing packages that linger on in the
     * status cache (That have a version of 0)
     *
     * @param states The package state(s) for which you wish to count packages for
     *
     * @return The total number of packages of the given PackageState in the Apt database
     */
    int packageCount(const Package::PackageStates &states) const;

    /**
     * Queries the backend for a list of all available packages, which is
     * essentially all packages, excluding now-nonexistent packages that have
     * a version of 0.
     *
     * \return A @c PackageList of all available packages in the Apt database
     */
    PackageList availablePackages() const;

    /**
     * Queries the backend for a list of all upgradeable packages
     *
     * \return A @c PackageList of all upgradeable packages in the Apt database
     */
    PackageList upgradeablePackages() const;

    /**
     * Queries the backend for a list of all packages that have been marked
     * for change. (To be installed, removed, etc)
     *
     * \return A @c PackageList of all marked packages in the Apt database
     */
    PackageList markedPackages() const;

    /**
     * Searches through the internal package list and returns a list of packages
     * based on the given input
     *
     * * @param searchString The string to narrow the search by
     *
     * \return A @c PackageList of all packages matching the search string
     */
    PackageList search(const QString &searchString) const;

    /**
     * Queries the backend for a Group object for the specified name
     *
     * @param name name used to specify the group returned
     *
     * @return A pointer to a @c Group defined by the specified name
     */
    Group *group(const QString &name) const;

    /**
     * Queries the backend for a list of all available groups
     *
     * \return A @c GroupList of all available groups in the Apt database
     */
    GroupList availableGroups() const;

    /**
     * Checks whether or not the search index needs updating
     *
     * \return @c true if the index needs updating
     * \return @c false if the index doesn't need updating
     */
    bool xapianIndexNeedsUpdate();

protected:
    BackendPrivate *const d_ptr;

    /**
     * Returns a pointer to the internal package source list. Mainly used for
     * internal purposes in QApt::Package.
     *
     * @return @c pkgSourceList The package source list used by the backend
     */
    pkgSourceList *packageSourceList();

private:
    Q_DECLARE_PRIVATE(Backend);
    friend class Package;

    /**
     * Prepares the package search mechanism
     *
     * \return @c true if opening succeeded
     * \return @c false if the opening didn't succeed
     */
    bool openXapianIndex();

    Package *package(pkgCache::PkgIterator &iter) const;

Q_SIGNALS:
    /**
     * Emitted whenever a backend error occurs. You should listen to this
     * signal and present the error/clean up when your app receives it.
     *
     * @param error QApt::ErrorCode enum member indicating error type
     * @param details A QVariant map containing info about the error, if available
     */
    void errorOccurred(QApt::ErrorCode error, const QVariantMap &details);

    void warningOccurred(QApt::WarningCode warning, const QVariantMap &details);

    /**
     * Emitted whenever the worker asks a question. You should listen to this
     * signal and present the question to the user when your app receives it.
     *
     * You should send the response back to the worker as a QVariantMap
     * using the Backend's answerWorkerQuestion() function.
     *
     * @param question A QApt::WorkerQuestion enum member indicating question type
     * @param details A QVariant map containing info about the question, if available
     */
    void questionOccurred(QApt::WorkerQuestion question, const QVariantMap &details);

    /**
     * Emitted whenever a package changes state. Useful for knowning when to
     * react to state changes.
     */
    void packageChanged();

    /**
     * Emitted whenever a backend event occurs.
     *
     * @param event A QApt::WorkerEvent enum member indicating event type
     */
    void workerEvent(QApt::WorkerEvent event);

    /**
     * Emitted while the QApt Worker is downloading packages.
     *
     * @param percentage Total percent complete
     * @param speed Current download speed in bytes
     * @param ETA Current estimated download time
     */
    void downloadProgress(int percentage, int speed, int ETA);

    /**
     * Emitted whenever an item has been downloaded
     *
     * @param flag Fetch type (is a QApt::Global enum member)
     * @param message Usually the URI of the item that's being downloaded
     */
    void downloadMessage(int flag, const QString &message);

    /**
     * Emits the progress of a current package installation/removal/
     * operation.
     *
     * @param status Current status retrieved from dpkg
     * @param percentage Total percent complete
     */
    void commitProgress(const QString &status, int percentage);

public Q_SLOTS:
    /**
     * Marks all upgradeable packages for upgrading, without marking new
     * packages for installation.
     */
    void markPackagesForUpgrade();

    /**
     * Marks all upgradeable packages for upgrading, including updates that
     * would require marking new packages for installation.
     */
    void markPackagesForDistUpgrade();

    /**
     * Marks a package for install.
     *
     * @param name The name of the package to be installed
     */
    void markPackageForInstall(const QString &name);

    /**
     * Marks a package for removal.
     *
     * @param name The name of the package to be removed
     */
    void markPackageForRemoval(const QString &name);

    /**
     * Commits all pending package state changes that have been made.
     */
    void commitChanges();

    /**
     * A slot that Packages use to tell the backend they've changed.
     * (Used internally by QApt::Package. You likely will never use this)
     */
    void packageChanged(Package *package);

    /**
     * Checks for and downloads new package source lists.
     */
    void updateCache();

    /**
     * Cancels download operations initialized by the updateCache() or
     * commitChanges() functions.
     */
    void cancelDownload();

    /**
     * This function should be used to return the answer the user has given
     * to a worker question. 
     */
    void answerWorkerQuestion(const QVariantMap &response);

private Q_SLOTS:
    void serviceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner);
    void workerStarted();
    void workerFinished(bool result);

    void emitErrorOccurred(int errorCode, const QVariantMap &details);
    void emitWarningOccurred(int warningCode, const QVariantMap &details);
    void emitWorkerEvent(int event);
    void emitWorkerQuestionOccurred(int question, const QVariantMap &details);
};

}

#endif
