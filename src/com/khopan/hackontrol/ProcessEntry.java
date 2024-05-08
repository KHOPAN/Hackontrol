package com.khopan.hackontrol;

public class ProcessEntry {
	public final int processIdentifier;
	public final int executionThreads;
	public final int parentProcessIdentifier;
	public final int threadBasePriority;
	public final String executableFile;

	private ProcessEntry(int processIdentifier, int executionThreads, int parentProcessIdentifier, int threadBasePriority, String executableFile) {
		this.processIdentifier = processIdentifier;
		this.executionThreads = executionThreads;
		this.parentProcessIdentifier = parentProcessIdentifier;
		this.threadBasePriority = threadBasePriority;
		this.executableFile = executableFile;
	}
}
