all:
	gcc	HW1_9101062141_Ser.c -o HW1_9101062141_Ser -lm
	gcc	HW1_9101062141_Cli.c -o HW1_9101062141_Cli -lm
clean:
	rm	-f HW1_9101062141_Cli
	rm	-f HW1_9101062141_Ser
