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
        desiredWidth = 260;
		addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "address", 10, 25);
		addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "port", 10, 45);
		addToggleParameterEditor(Parameter::PROCESSOR_SCOPE, "timestamp", 10, 65);
		addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "values", 10, 85);
		addTextBoxParameterEditor(Parameter::PROCESSOR_SCOPE, "sample_rate", 10, 105);
		sampleQualityComponent.setBounds(175, 65, SampleQualityComponent::width, SampleQualityComponent::height);
        addAndMakeVisible(&sampleQualityComponent);
		
		
	}


	DataThreadPluginEditor::~DataThreadPluginEditor() {

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

        // int would make more sense, but seems buggy rendering on windows (as of OE 1.0 at least)
        // https://forum.juce.com/t/int-vs-float-rectangles-with-scaling-a-proposal-for-windows-macos-consistency-for-juce-8/60744/8
        RectangleList<float> rectsGreen{};
        RectangleList<float> rectsRed{};
        RectangleList<float> rectsWhite{};

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


                    Rectangle<float> rect{
                        static_cast<float>(margin + (block == currentBlock ? nSeconds -1 : (nSeconds -1 + block - currentBlock) % nSeconds) * colWidth),
                        static_cast<float>(height - margin - (row + 1) * cellHeight),
                        static_cast<float>(colWidth),
                        static_cast<float>(cellHeight)};

                    if(v.dropped_super_early){
                        Rectangle<float> rect2 = rect;
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