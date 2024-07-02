package com.khopan.hackontrol.panel;

import com.khopan.hackontrol.registry.BiRegistrable;
import com.khopan.hackontrol.registry.Registrable;

import net.dv8tion.jda.api.entities.channel.concrete.TextChannel;

public abstract class Panel {
	protected TextChannel channel;

	PanelManager manager;

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
		if(this.manager == null) {
			throw new InternalError("Panel Manager is missing");
		}

		if(registrable == null) {
			throw new InternalError("Registration object cannot be null");
		}

		this.manager.register(registrable, value);
	}

	protected <T, U> void register(BiRegistrable<T, U> registrable, T key, U value) { 
		if(this.manager == null) {
			throw new InternalError("Panel Manager is missing");
		}

		if(registrable == null) {
			throw new InternalError("Registration object cannot be null");
		}

		this.manager.register(registrable, key, value);
	}
}
