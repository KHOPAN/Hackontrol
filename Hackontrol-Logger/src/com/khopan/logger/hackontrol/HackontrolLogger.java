package com.khopan.logger.hackontrol;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.Calendar;

import org.slf4j.Logger;
import org.slf4j.Marker;
import org.slf4j.helpers.MessageFormatter;

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
		return HackontrolLoggerConfig.Trace;
	}

	@Override
	public void trace(String message) {
		if(!HackontrolLoggerConfig.Trace) {
			return;
		}

		this.log(message, LogLevel.TRACE, null);
	}

	@Override
	public void trace(String format, Object argument) {
		if(!HackontrolLoggerConfig.Trace) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {argument}), LogLevel.TRACE, null);
	}

	@Override
	public void trace(String format, Object firstArgument, Object secondArgument) {
		if(!HackontrolLoggerConfig.Trace) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {firstArgument, secondArgument}), LogLevel.TRACE, null);
	}

	@Override
	public void trace(String format, Object... arguments) {
		if(!HackontrolLoggerConfig.Trace) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, arguments), LogLevel.TRACE, null);
	}

	@Override
	public void trace(String message, Throwable Errors) {
		if(!HackontrolLoggerConfig.Trace) {
			return;
		}

		this.log(message, LogLevel.TRACE, null);
		this.log(this.errorMessage(Errors), LogLevel.ERROR, null);
	}

	@Override
	public boolean isTraceEnabled(Marker marker) {
		return HackontrolLoggerConfig.Trace;
	}

	@Override
	public void trace(Marker marker, String message) {
		if(!HackontrolLoggerConfig.Trace) {
			return;
		}

		this.log(message, LogLevel.TRACE, marker);
	}

	@Override
	public void trace(Marker marker, String format, Object argument) {
		if(!HackontrolLoggerConfig.Trace) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {argument}), LogLevel.TRACE, marker);
	}

	@Override
	public void trace(Marker marker, String format, Object firstArgument, Object secondArgument) {
		if(!HackontrolLoggerConfig.Trace) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {firstArgument, secondArgument}), LogLevel.TRACE, marker);
	}

	@Override
	public void trace(Marker marker, String format, Object... arguments) {
		if(!HackontrolLoggerConfig.Trace) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, arguments), LogLevel.TRACE, marker);
	}

	@Override
	public void trace(Marker marker, String message, Throwable Errors) {
		if(!HackontrolLoggerConfig.Trace) {
			return;
		}

		this.log(message, LogLevel.TRACE, marker);
		this.log(this.errorMessage(Errors), LogLevel.ERROR, marker);
	}

	@Override
	public boolean isDebugEnabled() {
		return HackontrolLoggerConfig.Debug;
	}

	@Override
	public void debug(String message) {
		if(!HackontrolLoggerConfig.Debug) {
			return;
		}

		this.log(message, LogLevel.DEBUG, null);
	}

	@Override
	public void debug(String format, Object argument) {
		if(!HackontrolLoggerConfig.Debug) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {argument}), LogLevel.DEBUG, null);
	}

	@Override
	public void debug(String format, Object firstArgument, Object secondArgument) {
		if(!HackontrolLoggerConfig.Debug) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {firstArgument, secondArgument}), LogLevel.DEBUG, null);
	}

	@Override
	public void debug(String format, Object... arguments) {
		if(!HackontrolLoggerConfig.Debug) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, arguments), LogLevel.DEBUG, null);
	}

	@Override
	public void debug(String message, Throwable Errors) {
		if(!HackontrolLoggerConfig.Debug) {
			return;
		}

		this.log(message, LogLevel.DEBUG, null);
		this.log(this.errorMessage(Errors), LogLevel.ERROR, null);
	}

	@Override
	public boolean isDebugEnabled(Marker marker) {
		return HackontrolLoggerConfig.Debug;
	}

	@Override
	public void debug(Marker marker, String message) {
		if(!HackontrolLoggerConfig.Debug) {
			return;
		}

		this.log(message, LogLevel.DEBUG, marker);
	}

	@Override
	public void debug(Marker marker, String format, Object argument) {
		if(!HackontrolLoggerConfig.Debug) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {argument}), LogLevel.DEBUG, marker);
	}

	@Override
	public void debug(Marker marker, String format, Object firstArgument, Object secondArgument) {
		if(!HackontrolLoggerConfig.Debug) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {firstArgument, secondArgument}), LogLevel.DEBUG, marker);
	}

	@Override
	public void debug(Marker marker, String format, Object... arguments) {
		if(!HackontrolLoggerConfig.Debug) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, arguments), LogLevel.DEBUG, marker);
	}

	@Override
	public void debug(Marker marker, String message, Throwable Errors) {
		if(!HackontrolLoggerConfig.Debug) {
			return;
		}

		this.log(message, LogLevel.DEBUG, marker);
		this.log(this.errorMessage(Errors), LogLevel.ERROR, marker);
	}

	@Override
	public boolean isInfoEnabled() {
		return HackontrolLoggerConfig.Information;
	}

	@Override
	public void info(String message) {
		if(!HackontrolLoggerConfig.Information) {
			return;
		}

		this.log(message, LogLevel.INFO, null);
	}

	@Override
	public void info(String format, Object argument) {
		if(!HackontrolLoggerConfig.Information) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {argument}), LogLevel.INFO, null);
	}

	@Override
	public void info(String format, Object firstArgument, Object secondArgument) {
		if(!HackontrolLoggerConfig.Information) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {firstArgument, secondArgument}), LogLevel.INFO, null);
	}

	@Override
	public void info(String format, Object... arguments) {
		if(!HackontrolLoggerConfig.Information) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, arguments), LogLevel.INFO, null);
	}

	@Override
	public void info(String message, Throwable Errors) {
		if(!HackontrolLoggerConfig.Information) {
			return;
		}

		this.log(message, LogLevel.INFO, null);
		this.log(this.errorMessage(Errors), LogLevel.ERROR, null);
	}

	@Override
	public boolean isInfoEnabled(Marker marker) {
		return HackontrolLoggerConfig.Information;
	}

	@Override
	public void info(Marker marker, String message) {
		if(!HackontrolLoggerConfig.Information) {
			return;
		}

		this.log(message, LogLevel.INFO, marker);
	}

	@Override
	public void info(Marker marker, String format, Object argument) {
		if(!HackontrolLoggerConfig.Information) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {argument}), LogLevel.INFO, marker);
	}

	@Override
	public void info(Marker marker, String format, Object firstArgument, Object secondArgument) {
		if(!HackontrolLoggerConfig.Information) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {firstArgument, secondArgument}), LogLevel.INFO, marker);
	}

	@Override
	public void info(Marker marker, String format, Object... arguments) {
		if(!HackontrolLoggerConfig.Information) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, arguments), LogLevel.INFO, marker);
	}

	@Override
	public void info(Marker marker, String message, Throwable Errors) {
		if(!HackontrolLoggerConfig.Information) {
			return;
		}

		this.log(message, LogLevel.INFO, marker);
		this.log(this.errorMessage(Errors), LogLevel.ERROR, marker);
	}

	@Override
	public boolean isWarnEnabled() {
		return HackontrolLoggerConfig.Warning;
	}

	@Override
	public void warn(String message) {
		if(!HackontrolLoggerConfig.Warning) {
			return;
		}

		this.log(message, LogLevel.WARN, null);
	}

	@Override
	public void warn(String format, Object argument) {
		if(!HackontrolLoggerConfig.Warning) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {argument}), LogLevel.WARN, null);
	}

	@Override
	public void warn(String format, Object... arguments) {
		if(!HackontrolLoggerConfig.Warning) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, arguments), LogLevel.WARN, null);
	}

	@Override
	public void warn(String format, Object firstArgument, Object secondArgument) {
		if(!HackontrolLoggerConfig.Warning) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {firstArgument, secondArgument}), LogLevel.WARN, null);
	}

	@Override
	public void warn(String message, Throwable Errors) {
		if(!HackontrolLoggerConfig.Warning) {
			return;
		}

		this.log(message, LogLevel.WARN, null);
		this.log(this.errorMessage(Errors), LogLevel.ERROR, null);
	}

	@Override
	public boolean isWarnEnabled(Marker marker) {
		return HackontrolLoggerConfig.Warning;
	}

	@Override
	public void warn(Marker marker, String message) {
		if(!HackontrolLoggerConfig.Warning) {
			return;
		}

		this.log(message, LogLevel.WARN, marker);
	}

	@Override
	public void warn(Marker marker, String format, Object argument) {
		if(!HackontrolLoggerConfig.Warning) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {argument}), LogLevel.WARN, marker);
	}

	@Override
	public void warn(Marker marker, String format, Object firstArgument, Object secondArgument) {
		if(!HackontrolLoggerConfig.Warning) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {firstArgument, secondArgument}), LogLevel.WARN, marker);
	}

	@Override
	public void warn(Marker marker, String format, Object... arguments) {
		if(!HackontrolLoggerConfig.Warning) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, arguments), LogLevel.WARN, marker);
	}

	@Override
	public void warn(Marker marker, String message, Throwable Errors) {
		if(!HackontrolLoggerConfig.Warning) {
			return;
		}

		this.log(message, LogLevel.WARN, marker);
		this.log(this.errorMessage(Errors), LogLevel.ERROR, marker);
	}

	@Override
	public boolean isErrorEnabled() {
		return HackontrolLoggerConfig.Error;
	}

	@Override
	public void error(String message) {
		if(!HackontrolLoggerConfig.Error) {
			return;
		}

		this.log(message, LogLevel.ERROR, null);
	}

	@Override
	public void error(String format, Object argument) {
		if(!HackontrolLoggerConfig.Error) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {argument}), LogLevel.ERROR, null);
	}

	@Override
	public void error(String format, Object firstArgument, Object secondArgument) {
		if(!HackontrolLoggerConfig.Error) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {firstArgument, secondArgument}), LogLevel.ERROR, null);
	}

	@Override
	public void error(String format, Object... arguments) {
		if(!HackontrolLoggerConfig.Error) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, arguments), LogLevel.ERROR, null);
	}

	@Override
	public void error(String message, Throwable Errors) {
		if(!HackontrolLoggerConfig.Error) {
			return;
		}

		this.log(message, LogLevel.ERROR, null);
		this.log(this.errorMessage(Errors), LogLevel.ERROR, null);
	}

	@Override
	public boolean isErrorEnabled(Marker marker) {
		return HackontrolLoggerConfig.Error;
	}

	@Override
	public void error(Marker marker, String message) {
		if(!HackontrolLoggerConfig.Error) {
			return;
		}

		this.log(message, LogLevel.ERROR, marker);
	}

	@Override
	public void error(Marker marker, String format, Object argument) {
		if(!HackontrolLoggerConfig.Error) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {argument}), LogLevel.ERROR, marker);
	}

	@Override
	public void error(Marker marker, String format, Object firstArgument, Object secondArgument) {
		if(!HackontrolLoggerConfig.Error) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, new Object[] {firstArgument, secondArgument}), LogLevel.ERROR, marker);
	}

	@Override
	public void error(Marker marker, String format, Object... arguments) {
		if(!HackontrolLoggerConfig.Error) {
			return;
		}

		this.log(MessageFormatter.basicArrayFormat(format, arguments), LogLevel.ERROR, marker);
	}

	@Override
	public void error(Marker marker, String message, Throwable Errors) {
		if(!HackontrolLoggerConfig.Error) {
			return;
		}

		this.log(message, LogLevel.ERROR, marker);
		this.log(this.errorMessage(Errors), LogLevel.ERROR, null);
	}

	private void log(String message, LogLevel level, Marker marker) {
		if(message != null && (message.contains("\n") || message.contains("\r"))) {
			BufferedReader reader = new BufferedReader(new StringReader(message));
			String line;

			try {
				while((line = reader.readLine()) != null) {
					this.logInternal(line, level, marker);
				}
			} catch(IOException Exception) {
				this.logInternal(Exception.toString(), level, marker);
			}

			return;
		}

		this.logInternal(message, level, marker);
	}

	private void logInternal(String message, LogLevel level, Marker marker) {
		Calendar calendar = Calendar.getInstance();
		int hour = calendar.get(Calendar.HOUR_OF_DAY);
		int minute = calendar.get(Calendar.MINUTE);
		int second = calendar.get(Calendar.SECOND);
		String threadName = Thread.currentThread().getName();
		String type = level.name();
		String text = String.format("[%02d:%02d:%02d] [%s/%s] [%s%s]: %s", hour, minute, second, threadName, type, this.name, marker == null ? "" : "/" + marker.getName(), String.valueOf(message));

		if(LogLevel.WARN.equals(level) || LogLevel.ERROR.equals(level)) {
			HackontrolLoggerInternal.err.println(text);
		} else {
			HackontrolLoggerInternal.out.println(text);
		}
	}

	private String errorMessage(Throwable Errors) {
		if(Errors == null) {
			return null;
		}

		StringWriter stringWriter = new StringWriter();
		PrintWriter printWriter = new PrintWriter(stringWriter);
		Errors.printStackTrace(printWriter);
		String text = stringWriter.toString();
		printWriter.close();
		return text;
	}

	private static enum LogLevel {
		TRACE,
		DEBUG,
		INFO,
		WARN,
		ERROR;
	}
}
