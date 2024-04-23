package test;

import org.slf4j.Logger;

import com.khopan.hackontrol.logger.HackontrolLoggerFactory;

public class Main {
	public static final Logger LOGGER = new HackontrolLoggerFactory().getLogger("Main"); //LoggerFactory.getLogger("Main");

	public static void main(String[] args) {
		Main.LOGGER.info("Hello, world!");
		System.out.println("Standard Output");
		System.out.print(false);
		System.out.println();
		System.out.print('c');
		System.out.println();
		System.out.print(new char[] {'C', 'h', 'a', 'r', ' ', 'a', 'r', 'r', 'a', 'y'});
		System.out.println();
		System.out.print(21654.2164d);
		System.out.println();
		System.out.print(23.15f);
		System.out.println();
		System.out.print(216547431);
		System.out.println();
		System.out.print(217491579416576461L);
		System.out.println();
		System.out.print(new TestObject());
		System.out.println();
		System.out.print("Normal string?");
		System.out.println();
	}

	private static class TestObject extends Object {
		@Override
		public String toString() {
			return "I am an object";
		}
	}
}
