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
		
		
		// see note in header about being reactive to parameter changes
		for (ParameterEditor* ed : parameterEditors) {
			for (Component* c : ed->getChildren()) {
				if (Label* label = dynamic_cast<Label*>(c)) {
					label->addListener(this);
				}
			}
		}
		
	}


	void DataThreadPluginEditor::labelTextChanged(Label* labelThatHasChanged) {
		// see note in header about being reactive to parameter changes
		asyncUpdateSignalChain.triggerAsyncUpdate();
	}

	DataThreadPluginEditor::~DataThreadPluginEditor() {
		// see note in header about being reactive to parameter changes		
		for (ParameterEditor* ed : parameterEditors) {
			for (Component* c : ed->getChildren()) {
				if (Label* label = dynamic_cast<Label*>(c)) {
					label->removeListener(this);
				}
			}
		}
	}
}