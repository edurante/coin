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
  \class SoPolygonOffset SoPolygonOffset.h Inventor/nodes/SoPolygonOffset.h
  \brief The SoPolygonOffset class is a node type for "layering" rendering primitives.
  \ingroup nodes

  A common problem with realtime 3D rendering systems is that rendered
  primitives which are at approximately the same depth with regard to
  the camera viewpoint will appear to flicker. I.e.: from one angle
  one primitive will appear to be closer, while at another angle,
  another primitive will appear closer. When this happens, the
  rendered graphics at that part of the scene will of course look a
  lot less visually pleasing.

  One common situation where this problem often occurs is when you
  attempt to put a wireframe grid as an outline on top of filled
  polygons.

  The cause of the problem described above is that the Z-buffer of any
  render system has a limited resolution, often at 16, 24 or 32
  bits. Because of this, primitives which are close will sometimes get
  the \e same depth value in the Z-buffer, even though they are \a not
  actually at the same depth-coordinate.

  To rectify the flickering problem, this node can be inserted in the
  scene graph at the proper place(s) to explicitly define how
  polygons, lines and/or points should be offset with regard to other
  primitives.
*/

#include <Inventor/nodes/SoPolygonOffset.h>
#include <Inventor/nodes/SoSubNodeP.h>

#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoGLPolygonOffsetElement.h>
#include <Inventor/elements/SoOverrideElement.h>

/*!
  \enum SoPolygonOffset::Style

  Enumeration of the rendering primitives which can be influenced by
  an SoPolygonOffset node.
*/


/*!
  \var SoSFFloat SoPolygonOffset::factor

  Offset multiplication factor.
*/
/*!
  \var SoSFFloat SoPolygonOffset::units

  Absolute offset translation value.
*/
/*!
  \var SoSFBitMask SoPolygonOffset::styles

  The rendering primitive type to be influenced by this node. Defaults
  to SoPolygonOffset::FILLED.
*/
/*!
  \var SoSFBool SoPolygonOffset::on

  Whether the offset is on or off. Default is for SoPolygonOffset::on
  to be \c TRUE.
*/


// *************************************************************************

SO_NODE_SOURCE(SoPolygonOffset);

/*!
  Constructor.
*/
SoPolygonOffset::SoPolygonOffset(void)
{
  SO_NODE_INTERNAL_CONSTRUCTOR(SoPolygonOffset);

  SO_NODE_ADD_FIELD(factor, (1.0f));
  SO_NODE_ADD_FIELD(units, (1.0f));
  SO_NODE_ADD_FIELD(styles, (SoPolygonOffset::FILLED));
  SO_NODE_ADD_FIELD(on, (TRUE));

  SO_NODE_DEFINE_ENUM_VALUE(Style, FILLED);
  SO_NODE_DEFINE_ENUM_VALUE(Style, LINES);
  SO_NODE_DEFINE_ENUM_VALUE(Style, POINTS);
  SO_NODE_SET_SF_ENUM_TYPE(styles, Style);
}

/*!
  Destructor.
*/
SoPolygonOffset::~SoPolygonOffset()
{
}

void
SoPolygonOffset::initClass(void)
{
  SO_NODE_INTERNAL_INIT_CLASS(SoPolygonOffset);

  SO_ENABLE(SoCallbackAction, SoPolygonOffsetElement);
  SO_ENABLE(SoGLRenderAction, SoGLPolygonOffsetElement);
}


void
SoPolygonOffset::doAction(SoAction * action)
{
  SoState * state = action->getState();

  if (SoOverrideElement::getPolygonOffsetOverride(state)) return;

  float factorval, units_val;
  SoPolygonOffsetElement::Style styles_val;
  SbBool offset_on;

  SoPolygonOffsetElement::get(state, factorval, units_val, styles_val,
                              offset_on);
  if (!this->factor.isIgnored()) factorval = this->factor.getValue();
  if (!this->units.isIgnored()) units_val = this->units.getValue();
  if (!this->styles.isIgnored())
    styles_val = (SoPolygonOffsetElement::Style)this->styles.getValue();
  if (!this->on.isIgnored()) offset_on = this->on.getValue();

  SoPolygonOffsetElement::set(action->getState(),
                              this,
                              factorval,
                              units_val,
                              styles_val,
                              offset_on);

  if (this->isOverride()) {
    SoOverrideElement::setPolygonOffsetOverride(state, this, TRUE);
  }
}

void
SoPolygonOffset::callback(SoCallbackAction * action)
{
  SoPolygonOffset::doAction((SoAction *)action);
}

void
SoPolygonOffset::GLRender(SoGLRenderAction * action)
{
  SoPolygonOffset::doAction((SoAction *)action);
}
