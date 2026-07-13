#!/usr/bin/env ruby

module Tbdlang
        if ARGV[0] == "version"
                puts "Tbdlang - To Be Determined Language"
                puts "VM: Class 1"
                puts "Class: Interpreter"
                puts "Version: Alpha 1"
                puts "Lang: Ruby 4"
                puts "Suppoted Features: New, Mod, Func, Call, If, Escape, End"
                puts "Supported math: +, -, <, >, ==, !=, /, *"
                puts "Features supporting math: New, Mod, If"
                exit
        end

	@memory = {}
	@code = {}

	DEBUG = false
        ALLOW_ESCAPE = true # dangerous but needed for some modules

	def self.readfile(file)
		if File.exist?(file)
                        return File.read(file)
		else
			return nil
		end
	end

	def self.interpret_line(line)
		line = line.split(" ")

		case line[0]
		when "New", "Mod"
			return [line[0], line[1], line[3], line[4...line.index(")")]]
		when "Call"
			return [line[0], line[1]]
                when "Escape"
                        return [line[0], line[1..-1].join(" ")]
                when "If"
                  return [line[0], line[2...line.index(")")]]
		end
	end		

	def self.format_file(data)
		data = data.split("\n")
	end

	def self.run_cmd(command)
#		puts "DB: RUNNING #{command}" if DEBUG
		case command[0] 
		when "Mod"
			if @memory.key?(command[1])
				@memory[command[1]] = convert_data(command[2], command[3])
			else
				return "Error"
			end
		when "New"
			if @memory.key?(command[1])
				return "Error"
			else
				@memory[command[1]] = convert_data(command[2], command[3])
			end
		when "Call"
			@code[command[1]].each do |cmd|
				run_cmd(cmd)
			end
                when "Escape"
                        vm_escape(command)
                when "If"
                        if evaluate(command[1])
                                command[2].each do |cmd|
                                        run_cmd(cmd)
                                end
                        end
		end
	end

	def self.convert_data(type, data)
		output = []

		case type
		when "numb"
			output << evaluate(data)
		when "string"
			output = data
		end

		return output
	end

	def self.evaluate(expression)
		if expression.length == 1
			return expression[0].to_i
		end

                left = @memory.key?(expression[0]) ? @memory[expression[0]][0] : expression[0].to_i
                right = @memory.key?(expression[2]) ? @memory[expression[2]][0] : expression[2].to_i
		operator = expression[1]

		case operator
		when "+"
			return left + right
		when "-"
			return left - right
		when "*"
			return left * right
		when "/"
			return left / right
		when ">"
			return 1 if left > right
			return 0
		when "<"
			return 1 if left < right
			return 0
		when "=="
			return 1 if left == right
			return 0
                when "!="
                        return 1 if left != right
                        return 0
		else
			return expression
		end
	end

	def self.run(file)
		filedata = Tbdlang.readfile(file)
		formatted = Tbdlang.format_file(filedata)

		recording = false
		current_function = nil

                if_recording = false
                if_block = nil

		formatted.each do |line|
			next if line.strip.empty?
                        next if line.strip.start_with?("#")

			tokens = line.split(" ")
                        if if_recording
                                if tokens[0] == "End"
                                        puts "DB: END IF #{if_block}" if DEBUG

                                        run_cmd(if_block)

                                        if_recording = false
                                        if_block = nil
                                else
                                        command = interpret_line(line)

                                        unless command.nil?
                                                if_block[2] << command
                                                puts "DB: ADDED #{command} TO IF" if DEBUG
                                        end
                                end

                                next
                        end

			if recording
				if tokens[0] == "End"
					puts "DB: END OF FUNC #{current_function}" if DEBUG
					recording = false
					current_function = nil
				else
					command = interpret_line(line)
					@code[current_function] << interpret_line(line)
					puts "DB: ADDED #{command} TO #{current_function}" if DEBUG
				end

				next
			end

			case tokens[0]
			when "Func"
				recording = true
				current_function = tokens[2] # Func ( hello )
				@code[current_function] = []
				puts "DB: STARTED FUNC #{current_function}" if DEBUG
                        when "If"
                                recording = true if false
                                if_recording = true

                                condition = tokens[2...tokens.index(")")]
                                if_block = ["If", condition, []]

                                puts "DB: STARTED IF #{condition}" if DEBUG
			else
				command = interpret_line(line)
				next if command.nil?
				run_cmd(command)

				if DEBUG
					puts "DB: COMMAND: #{command}"
					puts "DB: MEMORY: #{@memory}"
					puts "DB: CODE: #{@code}"
				end
			end
		end
	end

        def self.vm_escape(tokens)
                if ALLOW_ESCAPE
                        token = tokens[1..-1].join(" ")
                        puts "VM: VM ESCAPE - #{token}" if DEBUG
                        eval(*token)
                else
                        error = "ERROR: VM ESCAPE DISABLED: TERMINATING PROGRAM"
                        puts error
                        abort(error)
                end
        end

	Tbdlang.run(ARGV[0])
end

