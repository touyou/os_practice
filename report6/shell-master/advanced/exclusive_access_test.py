#!/usr/bin/python
#
# exclusive_access_test:
#
# Test that the shell hands access over to a program
# correctly.
#
#       Requires the use of the following commands:
#
#       vim, ctrl-z control
#

import sys, imp, atexit
sys.path.append("/home/courses/software/pexpect-dpty/");
import pexpect, shellio, signal, time, os, re, proc_check

#Ensure the shell process is terminated
def force_shell_termination(shell_process):
        c.close(force=True)

#pulling in the regular expression and other definitions
definitions_scriptname = sys.argv[1]

def_module = imp.load_source('', definitions_scriptname)
logfile = None
if hasattr(def_module, 'logfile'):
    logfile = def_module.logfile

#spawn an instance of the shell
c = pexpect.spawn(def_module.shell, drainpty=True, logfile=logfile)
atexit.register(force_shell_termination, shell_process=c)

# set timeout for all following 'expect*' calls to 2 seconds
c.timeout = 2

# ensure that shell prints expected prompt
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"



# run a program requiring exclusive access
c.sendline("vi exclusive_vi")

# The following call is necessary to ensure that the SIGTSTP
# we are sending below via 'sendcontrol' reaches the 'sleep' child.
proc_check.wait_until_child_is_in_foreground(c)

#checks the number of active child processes
#using a timeout based process count
proc_check.count_children_timeout(c, 1, 1)

#checks the number of active child processes
#at this moment in time
proc_check.count_active_children(c, 1)

# send SIGTSTP to 'sleep'
c.sendcontrol('z')

# shell should pick up that 'sleep' was stopped and respond with job status
# it should output a line such as [6]+  Stopped                 (sleep 60)
# (note that the provided regexp assumes the job name appears in parentheses,
# adjust your eshoutput.py if needed)
(jobid, statusmsg, cmdline) = \
        shellio.parse_regular_expression(c, def_module.job_status_regex)
assert statusmsg == def_module.jobs_status_msg['stopped'], "Shell did not report stopped job"

# move job into foreground
c.sendline(def_module.builtin_commands['fg'] % jobid)

# exits
c.sendline(":quit");

# when moving a job in the foreground, bash outputs its command line
assert c.expect_exact(cmdline) == 0, "Shell did not report the job moved into the foreground"

#Add emacs in background
c.sendline("emacs &")

# pick up the background job output
(jobid, pid) = shellio.parse_regular_expression(c, def_module.bgjob_regex)

# check for prompt
assert c.expect(def_module.prompt) == 0, "Shell did not print expected prompt"

# check the jobs list
c.sendline(def_module.builtin_commands['jobs'])

# Check the status message
(jobid, status_message, command_line) = \
            shellio.parse_regular_expression(c, def_module.job_status_regex)

assert  (status_message == def_module.jobs_status_msg['stopped'])


#exit
c.sendline("exit")
assert c.expect_exact("exit") == 0, "Shell output extraneous characters"


shellio.success()
