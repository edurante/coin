/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  This file is part of the Coin library.
 *
 *  This file may be distributed under the terms of the Q Public License
 *  as defined by Troll Tech AS of Norway and appearing in the file
 *  LICENSE.QPL included in the packaging of this file.
 *
 *  If you want to use Coin in applications not covered by licenses
 *  compatible with the QPL, you can contact SIM to aquire a
 *  Professional Edition license for Coin.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

/*!
  \class SoGroup SoGroup.h Inventor/nodes/SoGroup.h
  \brief The SoGroup class ...
  \ingroup nodes

  FIXME: write class doc
*/

#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSubNodeP.h>
#include <assert.h>
#include <coindefs.h> // COIN_STUB()
#include <Inventor/SoInput.h>
#include <Inventor/SoOutput.h>
#include <Inventor/misc/SoChildList.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/errors/SoReadError.h>
#include <Inventor/actions/SoCallbackAction.h>


static const char * const DEFINITION_KEYWORD = "DEF";
static const char * const REFERENCE_KEYWORD  = "USE";

/*!
  \var SoChildList * SoGroup::children
  FIXME: write doc
*/

// *************************************************************************

SO_NODE_SOURCE(SoGroup);

/*!
  Constructor.
*/
SoGroup::SoGroup(void)
{
  SO_NODE_INTERNAL_CONSTRUCTOR(SoGroup);

  this->children = new SoChildList(this);
}

/*!
  FIXME: write function documentation
*/
SoGroup::SoGroup(int /* nchildren */)
{
  COIN_STUB();
}

/*!
  Destructor.
*/
SoGroup::~SoGroup()
{
  delete this->children;
}

/*!
  FIXME: write function documentation
*/
SoNode *
SoGroup::getChild(const int index) const
{
  assert((index >= 0) && (index < getNumChildren()));

  return (*this->children)[index];
}

/*!
  FIXME: write function documentation
*/
int
SoGroup::getNumChildren() const
{
  return this->children->getLength();
}

/*!
  FIXME: write function documentation
*/
SbBool
SoGroup::readInstance(SoInput * in, unsigned short flags)
{
  SbBool readfields = TRUE;

  // Make sure we're compatible with binary format Inventor 2.0 files.
  if (in->isBinary() && (in->getIVVersion() < 2.1f) &&
      this->getTypeId() == SoGroup::getClassTypeId())
    readfields = FALSE;

  // For nodes with fields inheriting SoGroup, the fields must come
  // before the children, according to the file format specification.
  if (readfields && !inherited::readInstance(in, flags)) return FALSE;

  return this->readChildren(in);
}

/*!
  Read all children of this node from \a in and attach them below this
  group in left-to-right order. Returns \c FALSE upon read error.
*/
SbBool
SoGroup::readChildren(SoInput * in)
{
  unsigned int numchildren = 0; // used by binary format import
  if (in->isBinary() && !in->read(numchildren)) {
    SoReadError::post(in, "Premature end of file");
    return FALSE;
  }

  for (unsigned int i=0; !in->isBinary() || (i < numchildren); i++) {
    SoBase * child;
    if (SoBase::read(in, child, SoNode::getClassTypeId())) {
      if (child == NULL) {
        if (in->eof()) {
          SoReadError::post(in, "Premature end of file");
          return FALSE;
        }
        else {
          if (in->isBinary()) {
            SoReadError::post(in, "Couldn't read valid identifier name");
            return FALSE;
          }

#if COIN_DEBUG && 0 // debug
          char m;
          if (in->read(m)) {
            SoDebugError::postInfo("SoGroup::readChildren",
                                   "next char: '%c'", m);
          }
#endif // debug
          // Completed reading of children for ASCII format import.
          return TRUE;
        }
      }
      else {
        this->addChild((SoNode *)child);
      }
    }
    else {
      // SoReadError::post() is called within the SoBase::read()
      // frame upon error conditions, so don't duplicate with
      // another error message here.  mortene.
      return FALSE;
    }
  }

  // A successful import operation for binary format reading of child
  // nodes will exit here.
  return TRUE;
}

// Overloaded from parent.
void
SoGroup::copyContents(const SoFieldContainer * from, SbBool copyconnections)
{
  this->removeAllChildren();

  inherited::copyContents(from, copyconnections);

  SoGroup * g = (SoGroup *)from;

  // Add children of "from" group node.
  for (int i=0 ; i < g->getNumChildren(); i++) {
    SoNode * cp = (SoNode *)
      SoFieldContainer::findCopy(g->getChild(i), copyconnections);
    this->addChild(cp);
  }
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::addChild(SoNode * const node)
{
  assert(node != NULL);
  this->children->append(node);
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::insertChild(SoNode * const child, const int newChildIndex)
{
  this->children->insert(child, newChildIndex);
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::removeChild(const int childIndex)
{
  this->children->remove(childIndex);
}

/*!
  FIXME: write function documentation
*/
int
SoGroup::findChild(const SoNode * const node) const
{
  return this->children->find((SoNode *) node);
}

/*!
  Does initialization common for all objects of the
  SoGroup class. This includes setting up the
  type system, among other things.
*/
void
SoGroup::initClass()
{
  SO_NODE_INTERNAL_INIT_CLASS(SoGroup);
}


/*!
  FIXME: write function documentation
*/
void
SoGroup::doAction(SoAction * action)
{
  int numIndices;
  const int * indices;
  switch (action->getPathCode(numIndices, indices)) {
  case SoAction::IN_PATH:
    // FIXME: not necessary to traverse children which do not
    // affect state and is not in indices[] ?
    // But, traversal will stop pretty soon anyway, so it might
    // be slower to include a check here. pederb, 19990618
    this->children->traverse(action, 0, indices[numIndices - 1]);
    break;

  case SoAction::NO_PATH:
  case SoAction::BELOW_PATH:
    this->children->traverse(action); // traverse all children
    break;

  case SoAction::OFF_PATH:
    {
      int n = this->getNumChildren();
      for (int i = 0; i < n; i++) {
        if (this->getChild(i)->affectsState())
          this->children->traverse(action, i);
      }
      break;
    }
  default:
    assert(0 && "Unknown path code");
    break;
  }
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::getBoundingBox(SoGetBoundingBoxAction * action)
{
  int numIndices;
  const int * indices;
  int lastChildIndex;

  if (action->getPathCode(numIndices, indices) == SoAction::IN_PATH)
    lastChildIndex = indices[numIndices-1];
  else
    lastChildIndex = getNumChildren() - 1;

  // Initialize accumulation variables.
  SbVec3f acccenter(0.0f, 0.0f, 0.0f);
  int numCenters = 0;

  for (int i = 0; i <= lastChildIndex; i++) {
    this->children->traverse(action, i);

    // If center point is set, accumulate.
    if (action->isCenterSet()) {
      acccenter += action->getCenter();
        numCenters++;
        action->resetCenter();
    }
  }

  if (numCenters != 0)
    action->setCenter(acccenter / float(numCenters), FALSE);
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::GLRender(SoGLRenderAction * action)
{
  SoGroup::doAction(action);
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::callback(SoCallbackAction * action)
{
  action->invokePreCallbacks(this);
  if (action->getCurrentResponse() == SoCallbackAction::CONTINUE) {
    SoGroup::doAction((SoAction *)action);
    action->invokePostCallbacks(this);
  }
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::getMatrix(SoGetMatrixAction * action)
{
  switch (action->getCurPathCode()) {
  case SoAction::NO_PATH:
  case SoAction::BELOW_PATH:
    break;
  case SoAction::OFF_PATH:
  case SoAction::IN_PATH:
    SoGroup::doAction((SoAction *)action);
    break;
  }
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::pick(SoPickAction * action)
{
  SoGroup::doAction((SoAction *)action);
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::handleEvent(SoHandleEventAction * action)
{
  SoGroup::doAction((SoAction *)action);
}

/*!
  Write action method is overloaded from SoNode to call
  SoBase::addWriteReference() on the children of the group.
*/
void
SoGroup::write(SoWriteAction * action)
{
  SoOutput * out = action->getOutput();
  if (out->getStage() == SoOutput::COUNT_REFS) {
    inherited::write(action);
    // Only increase number of writereferences to the top level node
    // in a tree which is used multiple times.
    if (!this->hasMultipleWriteRefs()) SoGroup::doAction((SoAction *)action);
  }
  else if (out->getStage() == SoOutput::WRITE) {
    if (this->writeHeader(out, TRUE, FALSE)) return;
    this->writeInstance(out);
    if (out->isBinary()) out->write(this->getNumChildren());
    SoGroup::doAction((SoAction *)action);
    this->writeFooter(out);
  }
  else assert(0 && "unknown stage");
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::search(SoSearchAction * action)
{
  // Include this node in the search.
  inherited::search(action);
  if (action->isFound()) return;

  // If we're not the one being sought after, try child subgraphs.
  SoGroup::doAction((SoAction *)action);
}

/*!
  FIXME: write function documentation
*/
SoChildList *
SoGroup::getChildren(void) const
{
  return ((SoGroup *)this)->children;
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::removeChild(SoNode * const child)
{
  int idx = this->findChild(child);
  if (idx >= 0) {
    this->removeChild(idx);
  }
  else {
    // FIXME: write some debug info
  }
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::removeAllChildren(void)
{
  this->children->truncate(0);
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::replaceChild(const int index, SoNode * const newChild)
{
  this->removeChild(index);
  this->insertChild(newChild, index);
}

/*!
  FIXME: write function documentation
*/
void
SoGroup::replaceChild(SoNode * const oldChild, SoNode * const newChild)
{
  this->replaceChild(this->findChild(oldChild), newChild);
}

/*!
  FIXME: write doc
*/
void
SoGroup::getPrimitiveCount(SoGetPrimitiveCountAction *action)
{
  SoGroup::doAction((SoAction*)action);
}
