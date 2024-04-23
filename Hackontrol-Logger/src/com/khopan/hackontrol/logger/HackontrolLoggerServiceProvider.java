package com.khopan.hackontrol.logger;

import org.slf4j.ILoggerFactory;
import org.slf4j.IMarkerFactory;
import org.slf4j.helpers.BasicMarkerFactory;
import org.slf4j.helpers.NOPMDCAdapter;
import org.slf4j.spi.MDCAdapter;
import org.slf4j.spi.SLF4JServiceProvider;

public class HackontrolLoggerServiceProvider implements SLF4JServiceProvider {
	public static String REQUESTED_API_VERSION = "2.0.99";

	private ILoggerFactory logger;
	private IMarkerFactory marker;
	private MDCAdapter adapter;

	@Override
	public ILoggerFactory getLoggerFactory() {
		return this.logger;
	}

	@Override
	public IMarkerFactory getMarkerFactory() {
		return this.marker;
	}

	@Override
	public MDCAdapter getMDCAdapter() {
		return this.adapter;
	}

	@Override
	public String getRequestedApiVersion() {
		return HackontrolLoggerServiceProvider.REQUESTED_API_VERSION;
	}

	@Override
	public void initialize() {
		this.logger = new HackontrolLoggerFactory();
		this.marker = new BasicMarkerFactory();
		this.adapter = new NOPMDCAdapter();
	}
}
