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
	DataThreadPluginEditor::DataThreadPluginEditor(GenericProcessor* parentNode, QualityInfo& qualityInfo)
		: GenericEditor(parentNode), sampleQualityComponent(qualityInfo)
	{
		desiredWidth = 280;
		addTextBoxParameterEditor("Address", 10, 75);
		addTextBoxParameterEditor("Port", 10, 25);
		addCheckBoxParameterEditor("Timestamp", 100, 25);
		addTextBoxParameterEditor("Values", 100, 75);
		addTextBoxParameterEditor("SampleRate", 190, 25);

		sampleQualityComponent.setBounds(190, 70, SampleQualityComponent::width, SampleQualityComponent::height);
        addAndMakeVisible(&sampleQualityComponent);
		
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


    SampleQualityComponent::SampleQualityComponent(QualityInfo& qualityInfo_):
        qualityInfo(qualityInfo_) {
            startTimer(5);
    };

    void SampleQualityComponent::timerCallback(){
         repaint();
    }

    void SampleQualityComponent::paint(Graphics& g){

        constexpr int margin = 1;
        constexpr int nSeconds = QualityInfo::bufferSeconds;
        constexpr int colWidth = (width - margin * 2) / nSeconds;

        RectangleList<int> rectsGreen{};
        RectangleList<int> rectsRed{};


        {
            const ScopedLock sl(qualityInfo.lock); // when reading from qualityInfo, need to lock it

            if(qualityInfo.buffer.size() > 0){

                int currentBlock = qualityInfo.bufferStartIdx / qualityInfo.sampleRate;
                int cellHeight = (height - margin*2) / qualityInfo.sampleRate;

                for(int i=0; i<qualityInfo.buffer.size(); i++){
                    int block = i / qualityInfo.sampleRate;
                    int row = i % qualityInfo.sampleRate;

                    if(block == currentBlock && i >= qualityInfo.bufferStartIdx){
                        continue; // ignore the tail end of the buffer
                    }

                    auto w  = colWidth - margin;
                    auto h = cellHeight - margin;
                    auto y = margin + row * cellHeight;
                    auto x = margin + ((nSeconds + block - currentBlock) % nSeconds) * colWidth;
                    auto v = qualityInfo.buffer[i];

                    // dummy display logic for now
                    if(i >= qualityInfo.bufferStartIdx){
                        rectsGreen.add(x, y, w, h);
                    } else {
                        rectsRed.add(x, y, w, h);
                    }
                }
            }
        }

        g.fillAll(Colours::grey);

        g.setColour(Colours::green);
        g.fillRectList(rectsGreen);

        g.setColour(Colours::red);
        g.fillRectList(rectsRed);
        //g.drawText(String(sampleRate) + " "  + String(bufferIndex), 10, 20, getWidth(), 20, Justification::centred);

    }
}