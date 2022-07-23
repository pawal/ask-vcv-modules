// BSD 2-Clause License
// Copyright (c) 2022, Patrik Wallström
#include "plugin.hpp"

struct Rnsh : Module {
	enum ParamId {
		PARAM1_PARAM, // spread knop
		PARAMS_LEN
	};
	enum InputId {
		GATE1_INPUT,
		GATE2_INPUT,
		GATE3_INPUT,
		GATE4_INPUT,
		GATE5_INPUT,
		GATE6_INPUT,
		GATE7_INPUT,
		GATE8_INPUT,
		INPUT1_INPUT, // cv spread input
		INPUTS_LEN
	};
	enum OutputId {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		OUT4_OUTPUT,
		OUT5_OUTPUT,
		OUT6_OUTPUT,
		OUT7_OUTPUT,
		OUT8_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	float g [8]  = {0,0,0,0,0,0,0,0}; // gate state
	float pg [8] = {0,0,0,0,0,0,0,0}; // previous gate state
	float spread = 0;
	dsp::SchmittTrigger triggers [8];
	dsp::SchmittTrigger trigger;

	Rnsh() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PARAM1_PARAM, 0.f, 1.f, 1.f, "Spread control voltage, multiplies all outputs");
		configInput(GATE1_INPUT, "Gate 1, triggers S&H");
		configInput(GATE2_INPUT, "Gate 2, triggers S&H");
		configInput(GATE3_INPUT, "Gate 3, triggers S&H");
		configInput(GATE4_INPUT, "Gate 4, triggers S&H");
		configInput(GATE5_INPUT, "Gate 5, triggers S&H");
		configInput(GATE6_INPUT, "Gate 6, triggers S&H");
		configInput(GATE7_INPUT, "Gate 7, triggers S&H");
		configInput(GATE8_INPUT, "Gate 8, triggers S&H");
		configInput(INPUT1_INPUT, "Spread control input (0 - 10V), multiplies all outputs");
		configOutput(OUT1_OUTPUT, "Outputs white noise if Gate 1 is not connected, S&H using the gate");
		configOutput(OUT2_OUTPUT, "Outputs white noise if Gate 2 is not connected, S&H using the gate");
		configOutput(OUT3_OUTPUT, "Outputs white noise if Gate 3 is not connected, S&H using the gate");
		configOutput(OUT4_OUTPUT, "Outputs white noise if Gate 4 is not connected, S&H using the gate");
		configOutput(OUT5_OUTPUT, "Outputs white noise if Gate 5 is not connected, S&H using the gate");
		configOutput(OUT6_OUTPUT, "Outputs white noise if Gate 6 is not connected, S&H using the gate");
		configOutput(OUT7_OUTPUT, "Outputs white noise if Gate 7 is not connected, S&H using the gate");
		configOutput(OUT8_OUTPUT, "Outputs white noise if Gate 8 is not connected, S&H using the gate");
	}

	void process(const ProcessArgs& args) override {
		// get the global spread multiplier
		if (inputs[INPUT1_INPUT].isConnected()) {
			spread = inputs[INPUT1_INPUT].getVoltage() / 10;
		} else {
			spread = params[PARAM1_PARAM].getValue();
		}
		// process all gates and outputs
		processGate(GATE1_INPUT, OUT1_OUTPUT, 0);
		processGate(GATE2_INPUT, OUT2_OUTPUT, 1);
		processGate(GATE3_INPUT, OUT3_OUTPUT, 2);
		processGate(GATE4_INPUT, OUT4_OUTPUT, 3);
		processGate(GATE5_INPUT, OUT5_OUTPUT, 4);
		processGate(GATE6_INPUT, OUT6_OUTPUT, 5);
		processGate(GATE7_INPUT, OUT7_OUTPUT, 6);
		processGate(GATE8_INPUT, OUT8_OUTPUT, 7);
	}

	// Process each gate, and set the output accordingly
	void processGate(int inputGate, int outputChannel, int Gstate) {
		if (inputs[inputGate].isConnected()) {
			// if gate state is set to high, output a new noise value to channel
			g[Gstate] = inputs[inputGate].getVoltage();
			g[Gstate] = triggers[Gstate].process(rescale(g[Gstate], 0.1f, 2.f, 0.f, 1.f));
			if (g[Gstate] > 0.5f && pg[Gstate] < 0.5f) {
				pg[Gstate] = g[Gstate];
				outputNoise(outputChannel);
			} else {
				pg[Gstate] = g[Gstate];
			}
		} else {
			// if gate is not connected, generate white noise for output
			if (outputs[outputChannel].isConnected()) {
				outputNoise(outputChannel);
			}
		}

	}

	// output random to the output channel
	void outputNoise(int outputChannel) {
		const float gain = 5.f / std::sqrt(2.f);
		if (outputs[outputChannel].isConnected()) {
			float white = random::normal();
			outputs[outputChannel].setVoltage(white * gain * spread);
		}
	}

};


struct RnshWidget : ModuleWidget {
	RnshWidget(Rnsh* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/rnsh.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.237, 102.821)), module, Rnsh::PARAM1_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.138, 24.625)), module, Rnsh::GATE1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.138, 33.773)), module, Rnsh::GATE2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.138, 43.389)), module, Rnsh::GATE3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.138, 52.536)), module, Rnsh::GATE4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.138, 61.918)), module, Rnsh::GATE5_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.138, 71.065)), module, Rnsh::GATE6_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.138, 80.564)), module, Rnsh::GATE7_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.138, 89.712)), module, Rnsh::GATE8_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.16, 102.821)), module, Rnsh::INPUT1_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.242, 24.625)), module, Rnsh::OUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.242, 33.773)), module, Rnsh::OUT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.242, 43.389)), module, Rnsh::OUT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.242, 52.536)), module, Rnsh::OUT4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.242, 61.918)), module, Rnsh::OUT5_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.242, 71.065)), module, Rnsh::OUT6_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.242, 80.564)), module, Rnsh::OUT7_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.242, 89.712)), module, Rnsh::OUT8_OUTPUT));
	}
};


Model* modelRnsh = createModel<Rnsh, RnshWidget>("rnsh");