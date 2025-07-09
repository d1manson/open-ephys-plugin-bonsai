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
	DataThreadPluginEditor::DataThreadPluginEditor(GenericProcessor* parentNode, DataThreadPlugin* thread, QualityInfo& qualityInfo)
		: GenericEditor(parentNode), sampleQualityComponent(qualityInfo)
	{
        this->thread = thread;

		desiredWidth = 280;
		addTextBoxParameterEditor(Parameter::GLOBAL_SCOPE, "Address", 10, 75);
		addTextBoxParameterEditor(Parameter::GLOBAL_SCOPE, "Port", 10, 25);
		addToggleParameterEditor(Parameter::GLOBAL_SCOPE, "Timestamp", 100, 25);
		addTextBoxParameterEditor(Parameter::GLOBAL_SCOPE, "Values", 100, 75);
		addTextBoxParameterEditor(Parameter::GLOBAL_SCOPE, "SampleRate", 190, 25);

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
    }

    void SampleQualityComponent::timerCallback(){
         repaint();
    }

    void SampleQualityComponent::paint(Graphics& g){

        constexpr int margin = 1;
        constexpr int nSeconds = QualityInfo::bufferSeconds;
        constexpr int colWidth = (width - margin * 2) / nSeconds;

        RectangleList<int> rectsGreen{};
        RectangleList<int> rectsRed{};
        RectangleList<int> rectsWhite{};

        {
            const ScopedLock sl(qualityInfo.lock); // when reading from qualityInfo, need to lock it

            if(qualityInfo.enabled){

                int currentBlock = qualityInfo.bufferStartIdx / qualityInfo.sampleRate;
                int cellHeight = std::max(1, (height - margin*2) / qualityInfo.sampleRate);

                for(int i=0; i<qualityInfo.buffer.size(); i++){
                    int block = i / qualityInfo.sampleRate;
                    int row = i % qualityInfo.sampleRate;

                    if(block == currentBlock && i >= qualityInfo.bufferStartIdx){
                        continue; // ignore the tail end of the buffer
                    }

                    auto v = qualityInfo.buffer[i];


                    Rectangle<int> rect{
                        margin + (block == currentBlock ? nSeconds -1 : (nSeconds -1 + block - currentBlock) % nSeconds) * colWidth,
                        height - margin - (row + 1) * cellHeight,
                        colWidth,
                        cellHeight};

                    if(v.dropped_super_early){
                        Rectangle<int> rect2 = rect;
                        rect2.setWidth(1);
                        rectsRed.addWithoutMerging(rect2);
                    }

                    if(v.filled_too_late){
                        rectsWhite.addWithoutMerging(rect);
                    } else if(v.used_value){
                        int reducedWidth = colWidth / 4 * v.error_size;
                        rect.setWidth(colWidth - reducedWidth);
                        rect.setX(rect.getX() + v.error_is_negative * reducedWidth);
                        rectsGreen.addWithoutMerging(rect);
                    }
                }
            }

        }

        g.fillAll(Colours::grey);

        g.setColour(Colours::green);
        g.fillRectList(rectsGreen);

        g.setColour(Colours::white);
        g.fillRectList(rectsWhite);

        g.setColour(Colours::red);
        g.fillRectList(rectsRed);
        //g.drawText(String(sampleRate) + " "  + String(bufferIndex), 10, 20, getWidth(), 20, Justification::centred);

    }
}