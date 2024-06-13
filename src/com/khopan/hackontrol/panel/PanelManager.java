package com.khopan.hackontrol.panel;

import java.util.ArrayList;
import java.util.List;

import net.dv8tion.jda.api.entities.channel.concrete.Category;

public class PanelManager {
	private final List<Panel> list;

	public PanelManager() {
		this.list = new ArrayList<>();
	}

	public void add(Panel panel) {
		this.list.add(panel);
	}

	public void start(Category category) {
		for(Panel panel : this.list) {

		}
	}
}
