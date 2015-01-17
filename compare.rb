#!/usr/bin/env ruby

EXECUTABLE = './sgm'
TEST_EXECUTABLE = './compare'
OUT_FILE = 'out.png'

class String
	def colorize(color_code)
		"\e[#{color_code}m#{self}\e[0m"
	end

	def bold
		"\033[1m#{self}\033[22m"
	end

	def red
		colorize(31)
	end

	def green
		colorize(32)
	end

	def yellow
		colorize(33)
	end
end

def run(cmd, args)
	command = "#{cmd} #{args.join(' ')}"
	puts command.bold
	ret = `#{command}`
	abort "Failed running command '#{command}'!" unless $?.success?
	ret
end

def print_result(match)
	match = 100 * match.to_f
	if match >= 95
		puts match.to_s.green
	elsif match >= 90
		puts match.to_s.yellow
	else
		puts match.to_s.red
	end
end

def test(left, right, range, ground)
	run(EXECUTABLE, [left, right, OUT_FILE, range])
	match = run(TEST_EXECUTABLE, [OUT_FILE, ground])
	print_result(match)
end

run('make', ['sgm', 'compare'])
test('test/bull/left.png', 'test/bull/right.png', 32, 'test/bull/ground.png')
test('test/venus/left.png', 'test/venus/right.png', 32, 'test/venus/ground.png')
# test('test/cones/left.png', 'test/cones/right.png', 64, 'test/cones/ground.png')
# test('test/teddy/left.png', 'test/teddy/right.png', 64, 'test/teddy/ground.png')