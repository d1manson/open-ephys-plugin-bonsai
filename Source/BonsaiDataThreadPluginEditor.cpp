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

#include "BonsaiDataThreadPluginEditor.h"



namespace Bonsai {
	DataThreadPluginEditor::DataThreadPluginEditor(GenericProcessor* parentNode)
		: GenericEditor(parentNode)
	{
		desiredWidth = 280;
		addTextBoxParameterEditor("Address", 10, 75);
		addTextBoxParameterEditor("Port", 10, 25);
		addCheckBoxParameterEditor("Timestamp", 100, 25);
		addTextBoxParameterEditor("Values", 100, 75);
		addTextBoxParameterEditor("SampleRate", 190, 25);

		sampleQualityComponent.setBounds(190, 70, 82, 50);
        addAndMakeVisible(&sampleQualityComponent);
        startTimer(5);
		
		// see note in header about being reactive to parameter changes
		for (ParameterEditor* ed : parameterEditors) {
			for (Component* c : ed->getChildren()) {
				if (Label* label = dynamic_cast<Label*>(c)) {
					label->addListener(this);
				} else if (ToggleButton* button = dynamic_cast<ToggleButton*>(c)) {
					button->addListener(this);
				}

			}
		}
		
	}


	void DataThreadPluginEditor::labelTextChanged(Label* src) {
		// see note in header about being reactive to parameter changes
		asyncUpdateSignalChain.triggerAsyncUpdate();
	}

	void DataThreadPluginEditor::buttonClicked(Button* src) {
		// see note in header about being reactive to parameter changes
		asyncUpdateSignalChain.triggerAsyncUpdate();
	}

    void DataThreadPluginEditor::setServer(OSCServer* server_){
         const ScopedLock sl(lock);
         server = server_;
    }

    void DataThreadPluginEditor::timerCallback(){
         const ScopedLock sl(lock);
         if(server != nullptr){
            server->copyQualityBuffer(sampleQualityComponent.buffer, sampleQualityComponent.bufferIndex, sampleQualityComponent.sampleRate);
         }
         sampleQualityComponent.repaint();
    }

	DataThreadPluginEditor::~DataThreadPluginEditor() {
		// see note in header about being reactive to parameter changes		
		for (ParameterEditor* ed : parameterEditors) {
			for (Component* c : ed->getChildren()) {
				if (Label* label = dynamic_cast<Label*>(c)) {
					label->removeListener(this);
				} else if (ToggleButton* button = dynamic_cast<ToggleButton*>(c)) {
					button->removeListener(this);
				}
			}
		}
	}

	SampleQualityComponent::SampleQualityComponent(){
	}
	SampleQualityComponent::~SampleQualityComponent(){
    }
    void SampleQualityComponent::paint(Graphics& g){
        g.fillAll(Colours::black);
        g.setColour(Colours::white);


        g.drawText(String(sampleRate) + " "  + String(bufferIndex), 10, 20, getWidth(), 20, Justification::centred);
        /*for(int i=0; i<10; i++){
            auto v = buffer[(bufferIndex + i) % buffer.size()];
            g.drawText(String((bufferIndex + i) % buffer.size()), i*10, 20, getWidth(), 20, Justification::centred);
        }*/
    }
}