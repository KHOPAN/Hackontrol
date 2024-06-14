package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.registry.BiRegistrable;
import com.khopan.hackontrol.registry.Registrable;
import com.khopan.hackontrol.widget.ControlWidget;

import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;

public abstract class Panel {
	protected TextChannel channel;

	public Panel() {

	}

	public abstract String panelName();

	public void registeration() {

	}

	public ControlWidget[] controlWidget() {
		return null;
	}

	public void initialize() {

	}

	protected <T> void register(Registrable<T> registrable, T value) {

	}

	protected <T, U> void register(BiRegistrable<T, U> registrable, T key, U value) { 

	}
}
