/*
 *   Copyright 2011 Marco Martin <notmart@gmail.com>
 *   Copyright 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: notificationItem
    width: parent.width
    implicitHeight: {
        var absoluteMinimum = actionsColumn.height + closeButton.height + 3 * units.smallSpacing
        return compact ? Math.max(absoluteMinimum, closeButton.height + units.smallSpacing + textItemLoader.item.height) : Math.max(absoluteMinimum, 4.5 * units.gridUnit)
    }

    signal close
    signal configure
    signal action(string actionId)

    property alias textItem: textItemLoader.sourceComponent
    property bool compact: false

    property alias icon: appIconItem.icon
    property alias image: imageItem.image
    property alias summary: summaryLabel.text
    property alias configurable: settingsButton.visible

    property ListModel actions: ListModel { }

    QIconItem {
        id: appIconItem

        width: units.iconSizes.large
        height: units.iconSizes.large

        visible: !imageItem.visible
    }

    QImageItem {
        id: imageItem
        anchors.fill: appIconItem

        smooth: true
        visible: nativeWidth > 0
    }

    RowLayout {
        id: titleBar
        anchors {
            top: parent.top
            left: appIconItem.right
            right: parent.right
            leftMargin: units.smallSpacing * 2
        }
        spacing: units.smallSpacing

        PlasmaExtras.Heading {
            id: summaryLabel
            Layout.fillWidth: true
            level: 4
            height: paintedHeight
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
        }

        PlasmaComponents.ToolButton {
            id: settingsButton
            width: units.iconSizes.smallMedium
            height: width
            visible: false

            iconSource: "configure"

            onClicked: configure()
        }

        PlasmaComponents.ToolButton {
            id: closeButton
            width: units.iconSizes.smallMedium
            height: width
            flat: compact

            iconSource: "window-close"

            onClicked: close()
        }
    }

    Loader {
        id: textItemLoader
        anchors {
            top: titleBar.bottom
            left: appIconItem.right
            right: actionsColumn.visible ? actionsColumn.left : parent.right
            bottom: parent.bottom
            bottomMargin: units.smallSpacing
            leftMargin: units.smallSpacing * 2
            rightMargin: units.smallSpacing * 2
        }
    }

    Column {
        id: actionsColumn
        anchors {
            top: titleBar.bottom
            right: parent.right
            topMargin: units.smallSpacing
        }
        height: childrenRect.height
        spacing: units.smallSpacing
        visible: notificationItem.actions.count > 0

        Repeater {
            id: actionRepeater
            model: notificationItem.actions

            PlasmaComponents.Button {
                width: theme.mSize(theme.defaultFont).width * (compact ? 8 : 12)
                text: model.text
                onClicked: notificationItem.action(model.id)
            }
        }
    }

}