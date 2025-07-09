/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2022 Open Ephys

 ------------------------------------------------------------------

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef DATATHREADPLUGINEDITOR_H_DEFINED
#define DATATHREADPLUGINEDITOR_H_DEFINED

#include <EditorHeaders.h>
#include <ProcessorHeaders.h>
#include "QualityInfo.h"
#include "BonsaiDataThreadPlugin.h"

namespace Bonsai {


    class SampleQualityComponent : public Component, Timer {
    public:
        SampleQualityComponent(QualityInfo& qualityInfo_);
        ~SampleQualityComponent() {};

        QualityInfo& qualityInfo;

        void timerCallback() override;
        void paint(Graphics& g) override;

        constexpr static int width = 82;
        constexpr static int height = 50;
    private:
        /** Generates an assertion if this class leaks */
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleQualityComponent);
    };



	// Note on being reactive to parameter changes..
	// Something about the DataThread class doesn't automatically support updating the signal chain when a parameter is changed.
	// But after a lot of faff, I found a way to add listeners to the Label/ToggleButon instances that are the parameter editors, and then
	// I also found that you can't call CoreServices::updateSignalChain(this) immediately (it seems to stop the edit actually happening).
	// However, it seems that you can make that call via an AsyncUpdater. So as a result, the code includes:
	//	a) logic in the editor constructor to register listners on all the paramater labels
	//  b) logic in the editor destructor ro remove those listners (not entirely sure if that's strictly neccessary, but it's recommended somewhere).
	//  c) a AsyncUpdateSignalChain class that can hold a reference to the editor and make the call.
	//  d) an instance of that class on the editor
	//  e) an additional base class on the editor, Label::Listener, and an implementation of labelTextChanged method which invokes the asyncUpdateSignalChain,
	//    and similar for Buttons (toggle buttons).
	// It's possible the SourceNode will be changed to make this easier somehow in future, in which case all that can be deleted!
	class AsyncUpdateSignalChain : public AsyncUpdater {
	public:
		AsyncUpdateSignalChain(GenericEditor* editor_) : editor(editor_) {};
		~AsyncUpdateSignalChain() { editor = nullptr; };
		void handleAsyncUpdate() { CoreServices::updateSignalChain(editor); };
	private:
		GenericEditor* editor;
	};




	class DataThreadPluginEditor : public GenericEditor, Label::Listener, Button::Listener
	{
	public:
		/** The class constructor, used to initialize any members. */
		DataThreadPluginEditor (GenericProcessor* parentNode, DataThreadPlugin* thread, QualityInfo& qualityInfo);

		/** The class destructor, used to deallocate memory */
		~DataThreadPluginEditor();

		void labelTextChanged(Label*) override;
		void buttonClicked(Button*)  override;

	private:

		/** A pointer to the underlying DataThreadPlugin */
    	DataThreadPlugin* thread;

		AsyncUpdateSignalChain asyncUpdateSignalChain = { this };

        SampleQualityComponent sampleQualityComponent;

    };

}
#endif