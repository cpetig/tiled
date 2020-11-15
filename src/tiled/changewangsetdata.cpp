/*
 * changewangsetdata.cpp
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "changewangsetdata.h"

#include "changeevents.h"
#include "changetilewangid.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "tilesetwangsetmodel.h"

#include <QCoreApplication>

#include "qtcompat_p.h"

using namespace Tiled;

RenameWangSet::RenameWangSet(TilesetDocument *tilesetDocument,
                             WangSet *wangSet,
                             const QString &newName)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set Name"))
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mOldName(wangSet->name())
    , mNewName(newName)
{
}

void RenameWangSet::undo()
{
    mTilesetDocument->wangSetModel()->setWangSetName(mWangSet, mOldName);
}

void RenameWangSet::redo()
{
    mTilesetDocument->wangSetModel()->setWangSetName(mWangSet, mNewName);
}


ChangeWangSetType::ChangeWangSetType(TilesetDocument *tilesetDocument,
                                     WangSet *wangSet,
                                     WangSet::Type newType,
                                     QUndoCommand *parent)
    : QUndoCommand(parent)
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mOldType(wangSet->type())
    , mNewType(newType)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Wang Set Type"));
}

void ChangeWangSetType::undo()
{
    mTilesetDocument->wangSetModel()->setWangSetType(mWangSet, mOldType);
}

void ChangeWangSetType::redo()
{
    mTilesetDocument->wangSetModel()->setWangSetType(mWangSet, mNewType);
}


ChangeWangSetColorCount::ChangeWangSetColorCount(TilesetDocument *tilesetDocument,
                                                 WangSet *wangSet,
                                                 int newValue)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set Color Count"))
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mOldValue(wangSet->colorCount())
    , mNewValue(newValue)
{
    // when edge size changes, all tiles with WangIds need to be updated.
    if (mNewValue < mOldValue) {
        // when the size is reduced, some Wang assignments can be lost.
        const auto changes = ChangeTileWangId::changesOnSetColorCount(wangSet, mNewValue);
        if (!changes.isEmpty())
            new ChangeTileWangId(mTilesetDocument, wangSet, changes, this);

        for (int i = mOldValue; i > mNewValue; --i) {
            WangColorChange w;
            w.index = i;
            w.wangColor = wangSet->colorAt(i);

            mRemovedWangColors.append(w);
        }
    }
}

void ChangeWangSetColorCount::undo()
{
    mTilesetDocument->wangSetModel()->setWangSetColorCount(mWangSet, mOldValue);

    for (const WangColorChange &w : qAsConst(mRemovedWangColors)) {
        WangColor &wangColor = *mWangSet->colorAt(w.index);
        wangColor.setName(w.wangColor->name());
        wangColor.setImageId(w.wangColor->imageId());
        wangColor.setColor(w.wangColor->color());
        wangColor.setProbability(w.wangColor->probability());
    }

    QUndoCommand::undo();
}

void ChangeWangSetColorCount::redo()
{
    mTilesetDocument->wangSetModel()->setWangSetColorCount(mWangSet, mNewValue);

    QUndoCommand::redo();
}


ChangeWangSetFlipping::ChangeWangSetFlipping(TilesetDocument *TilesetDocument,
                                             WangSet *wangSet,
                                             ChangeType _which,
                                             bool newValue)
    : mTilesetDocument(TilesetDocument), mWangSet(wangSet), mWhich(_which),
      mOldValue(_which == flipX ? wangSet->asNeededFlipHorizontally()
              : _which == flipY ? wangSet->asNeededFlipVertically()
              : _which == flipAD ? wangSet->asNeededFlipAntiDiagonally()
                                 : wangSet->randomizeOrientation()),
      mNewValue(newValue)
{
}

void ChangeWangSetFlipping::undo()
{
    switch (mWhich)
    {
    case flipX: mTilesetDocument->wangSetModel()->setAsNeededFlipHorizontally(mWangSet, mOldValue); break;
    case flipY: mTilesetDocument->wangSetModel()->setAsNeededFlipVertically(mWangSet, mOldValue); break;
    case flipAD: mTilesetDocument->wangSetModel()->setAsNeededFlipAntiDiagonally(mWangSet, mOldValue); break;
    case randomFlip: mTilesetDocument->wangSetModel()->setRandomizeOrientation(mWangSet, mOldValue); break;
    }
    QUndoCommand::undo();
}
void ChangeWangSetFlipping::redo()
{
    switch (mWhich)
    {
    case flipX: mTilesetDocument->wangSetModel()->setAsNeededFlipHorizontally(mWangSet, mNewValue); break;
    case flipY: mTilesetDocument->wangSetModel()->setAsNeededFlipVertically(mWangSet, mNewValue); break;
    case flipAD: mTilesetDocument->wangSetModel()->setAsNeededFlipAntiDiagonally(mWangSet, mNewValue); break;
    case randomFlip: mTilesetDocument->wangSetModel()->setRandomizeOrientation(mWangSet, mNewValue); break;
    }
    QUndoCommand::redo();
}


RemoveWangSetColor::RemoveWangSetColor(TilesetDocument *tilesetDocumnet, WangSet *wangSet, int color)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Remove Wang Color"))
    , mTilesetDocument(tilesetDocumnet)
    , mWangSet(wangSet)
    , mColor(color)
{
    mRemovedWangColor = wangSet->colorAt(mColor);

    const auto changes = ChangeTileWangId::changesOnRemoveColor(wangSet, color);
    if (!changes.isEmpty())
        new ChangeTileWangId(mTilesetDocument, wangSet, changes, this);
}

void RemoveWangSetColor::undo()
{
    mTilesetDocument->wangSetModel()->insertWangColor(mWangSet, mRemovedWangColor);

    QUndoCommand::undo();
}

void RemoveWangSetColor::redo()
{
    mTilesetDocument->wangSetModel()->removeWangColorAt(mWangSet, mColor);

    QUndoCommand::redo();
}


SetWangSetImage::SetWangSetImage(TilesetDocument *tilesetDocument,
                                 WangSet *wangSet,
                                 int tileId,
                                 QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Set Wang Set Image"),
                   parent)
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mOldImageTileId(wangSet->imageTileId())
    , mNewImageTileId(tileId)
{
}

void SetWangSetImage::undo()
{
    mTilesetDocument->wangSetModel()->setWangSetImage(mWangSet, mOldImageTileId);
}

void SetWangSetImage::redo()
{
    mTilesetDocument->wangSetModel()->setWangSetImage(mWangSet, mNewImageTileId);
}
