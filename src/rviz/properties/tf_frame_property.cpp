/*
 * Copyright (c) 2012, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "rviz/frame_manager.h"

#include "rviz/properties/tf_frame_property.h"

namespace rviz
{

const QString TfFrameProperty::FIXED_FRAME_STRING = "<Fixed Frame>";

TfFrameProperty::TfFrameProperty( const QString& name,
                                  const QString& default_value,
                                  const QString& description,
                                  Property* parent,
                                  FrameManager* frame_manager,
                                  bool include_fixed_frame_string,
                                  const char *changed_slot,
                                  QObject* receiver )
  : EditableEnumProperty( name, default_value, description, parent, changed_slot, receiver )
  , frame_manager_( NULL )
  , include_fixed_frame_string_( include_fixed_frame_string )
{
  // Parent class EditableEnumProperty has requestOptions() signal.
  connect( this, SIGNAL( requestOptions( QStringList* )),
           this, SLOT( fillFrameList( QStringList* )));
  setFrameManager( frame_manager );
}

bool TfFrameProperty::setValue( const QVariant& new_value )
{
  QString new_string = new_value.toString();
  if( new_string != FIXED_FRAME_STRING )
  {
    new_string = QString::fromStdString( frame_manager_->getTFClient()->resolve( new_string.toStdString() ));
  }
  bool result = EditableEnumProperty::setValue( new_string );

  return result;
}

void TfFrameProperty::setFrameManager( FrameManager* frame_manager )
{
  if( frame_manager_ && include_fixed_frame_string_ )
  {
    disconnect( frame_manager_, SIGNAL( fixedFrameChanged() ),
                this, SLOT( handleFixedFrameChange() ));
  }
  frame_manager_ = frame_manager;
  if( frame_manager_ && include_fixed_frame_string_ )
  {
    connect( frame_manager_, SIGNAL( fixedFrameChanged() ),
             this, SLOT( handleFixedFrameChange() ));
  }
}

void TfFrameProperty::fillFrameList( QStringList* frame_list_return )
{
  if( !frame_list_return )
  {
    return;
  }
  std::vector<std::string> std_frames;
  frame_manager_->getTFClient()->getFrameStrings( std_frames );

  for( size_t i = 0; i < std_frames.size(); i++ )
  {
    frame_list_return->append( QString::fromStdString( std_frames[ i ]));
  }

  frame_list_return->sort();
  if( include_fixed_frame_string_ )
  {
    frame_list_return->push_front( FIXED_FRAME_STRING );
  }
}

QString TfFrameProperty::getFrame() const
{
  QString frame = getValue().toString();
  if( frame == FIXED_FRAME_STRING && frame_manager_ )
  {
    return QString::fromStdString( frame_manager_->getFixedFrame() );
  }
  return frame;
}

std::string TfFrameProperty::getFrameStd() const
{
  return getFrame().toStdString();
}

void TfFrameProperty::handleFixedFrameChange()
{
  if( getValue().toString() == FIXED_FRAME_STRING )
  {
    Q_EMIT changed();
  }
}

} // end namespace rviz