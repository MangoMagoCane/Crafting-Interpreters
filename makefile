JLOX_PATH := jlox/classes

build_jlox jlox/Lox.java:
	javac -d $(JLOX_PATH) jlox/Lox.java

run_jlox:
	java -cp $(JLOX_PATH) jlox.Lox $(ARGS)

exec_jlox: build_jlox run_jlox
