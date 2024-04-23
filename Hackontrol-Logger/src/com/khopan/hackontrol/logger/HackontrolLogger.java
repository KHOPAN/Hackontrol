package com.khopan.hackontrol.logger;

import java.util.Calendar;

import org.slf4j.Logger;
import org.slf4j.Marker;

public class HackontrolLogger implements Logger {
	static {
		HackontrolLoggerInternal.staticInitialize();
	}

	private final String name;

	HackontrolLogger(String name) {
		this.name = name;
	}

	@Override
	public String getName() {
		return this.name;
	}

	@Override
	public boolean isTraceEnabled() {
		return false;
	}

	@Override
	public void trace(String message) {

	}

	@Override
	public void trace(String format, Object argument) {

	}

	@Override
	public void trace(String format, Object firstArgument, Object secondArgument) {

	}

	@Override
	public void trace(String format, Object... arguments) {

	}

	@Override
	public void trace(String message, Throwable Errors) {

	}

	@Override
	public boolean isTraceEnabled(Marker marker) {
		return false;
	}

	@Override
	public void trace(Marker marker, String message) {

	}

	@Override
	public void trace(Marker marker, String format, Object argument) {

	}

	@Override
	public void trace(Marker marker, String format, Object firstArgument, Object secondArgument) {

	}

	@Override
	public void trace(Marker marker, String format, Object... arguments) {

	}

	@Override
	public void trace(Marker marker, String message, Throwable Errors) {

	}

	@Override
	public boolean isDebugEnabled() {
		return false;
	}

	@Override
	public void debug(String message) {

	}

	@Override
	public void debug(String format, Object argument) {

	}

	@Override
	public void debug(String format, Object firstArgument, Object secondArgument) {

	}

	@Override
	public void debug(String format, Object... arguments) {

	}

	@Override
	public void debug(String message, Throwable Errors) {

	}

	@Override
	public boolean isDebugEnabled(Marker marker) {
		return false;
	}

	@Override
	public void debug(Marker marker, String message) {

	}

	@Override
	public void debug(Marker marker, String format, Object argument) {

	}

	@Override
	public void debug(Marker marker, String format, Object firstArgument, Object secondArgument) {

	}

	@Override
	public void debug(Marker marker, String format, Object... arguments) {

	}

	@Override
	public void debug(Marker marker, String message, Throwable Errors) {

	}

	@Override
	public boolean isInfoEnabled() {
		return true;
	}

	@Override
	public void info(String message) {
		this.log(message, LogLevel.INFO);
	}

	@Override
	public void info(String format, Object argument) {

	}

	@Override
	public void info(String format, Object firstArgument, Object secondArgument) {

	}

	@Override
	public void info(String format, Object... arguments) {

	}

	@Override
	public void info(String message, Throwable Errors) {

	}

	@Override
	public boolean isInfoEnabled(Marker marker) {
		return false;
	}

	@Override
	public void info(Marker marker, String message) {

	}

	@Override
	public void info(Marker marker, String format, Object argument) {

	}

	@Override
	public void info(Marker marker, String format, Object firstArgument, Object secondArgument) {

	}

	@Override
	public void info(Marker marker, String format, Object... arguments) {

	}

	@Override
	public void info(Marker marker, String message, Throwable Errors) {

	}

	@Override
	public boolean isWarnEnabled() {
		return false;
	}

	@Override
	public void warn(String message) {

	}

	@Override
	public void warn(String format, Object argument) {

	}

	@Override
	public void warn(String format, Object... arguments) {

	}

	@Override
	public void warn(String format, Object firstArgument, Object secondArgument) {

	}

	@Override
	public void warn(String message, Throwable Errors) {

	}

	@Override
	public boolean isWarnEnabled(Marker marker) {
		return false;
	}

	@Override
	public void warn(Marker marker, String message) {

	}

	@Override
	public void warn(Marker marker, String format, Object argument) {

	}

	@Override
	public void warn(Marker marker, String format, Object firstArgument, Object secondArgument) {

	}

	@Override
	public void warn(Marker marker, String format, Object... arguments) {

	}

	@Override
	public void warn(Marker marker, String message, Throwable Errors) {

	}

	@Override
	public boolean isErrorEnabled() {
		return false;
	}

	@Override
	public void error(String message) {

	}

	@Override
	public void error(String format, Object argument) {

	}

	@Override
	public void error(String format, Object firstArgument, Object secondArgument) {

	}

	@Override
	public void error(String format, Object... arguments) {

	}

	@Override
	public void error(String message, Throwable Errors) {

	}

	@Override
	public boolean isErrorEnabled(Marker marker) {
		return false;
	}

	@Override
	public void error(Marker marker, String message) {

	}

	@Override
	public void error(Marker marker, String format, Object argument) {

	}

	@Override
	public void error(Marker marker, String format, Object firstArgument, Object secondArgument) {

	}

	@Override
	public void error(Marker marker, String format, Object... arguments) {

	}

	@Override
	public void error(Marker marker, String message, Throwable Errors) {

	}

	private void log(String message, LogLevel level) {
		Calendar calendar = Calendar.getInstance();
		int hour = calendar.get(Calendar.HOUR_OF_DAY);
		int minute = calendar.get(Calendar.MINUTE);
		int second = calendar.get(Calendar.SECOND);
		String threadName = Thread.currentThread().getName();
		String type = level.name();
		HackontrolLoggerInternal.out.println(String.format("[%02d:%02d:%02d] [%s/%s] [%s]: %s", hour, minute, second, threadName, type, this.name, message));
	}

	private static enum LogLevel {
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR;
	}
}
