JLOX_PATH := classes

build lox/Lox.java:
	javac -d $(JLOX_PATH) lox/Lox.java

run:
	java -cp $(JLOX_PATH) lox.Lox $(ARGS)

gen_ast:
	javac -d $(JLOX_PATH) tool/GenerateAst.java
	java -cp $(JLOX_PATH) tool.GenerateAst lox

exec: build run
