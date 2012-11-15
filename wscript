from waflib.TaskGen import feature, before, after
from waflib.Task import Task
from waflib.Configure import conf
from waflib import Utils

APPNAME = 'os'
VERSION = '1.0'

out = 'out'

def options(ctx):
	ctx.load('gcc gxx')

def configure(ctx):
	ctx.setenv('host')
	ctx.load('gcc gxx')

	ctx.setenv('cross')
	ctx.find_program('arm-none-eabi-gcc', var='CC')
	ctx.find_program('arm-none-eabi-g++', var='CXX')
	ctx.find_program('arm-none-eabi-as',  var='AS')
	ctx.find_program('arm-none-eabi-objcopy', var='OBJCOPY')

	ctx.load('gcc gxx gas')
	ctx.env.append_value('CFLAGS', ['-g', '-Werror'])
	ctx.env.append_value('CXXFLAGS', ['-g', '-fno-exceptions', '-fno-rtti', '-Werror'])
	ctx.env.cxxprogram_PATTERN = '%s'
	ctx.env.LINKFLAGS = ''

@conf
def crossprogram(ctx, *k, **kw):
	kw['includes'] = Utils.to_list(kw.get('includes', []))
	kw['includes'] += [ctx.top_dir]
	ctx.program(env=ctx.all_envs['cross'].derive(), *k, **kw)

@conf
def crossstlib(ctx, *k, **kw):
	kw['includes'] = Utils.to_list(kw.get('includes', []))
	kw['includes'] += [ctx.top_dir]
	ctx.stlib(env=ctx.all_envs['cross'].derive(), *k, **kw)

@conf
def crossobjects(ctx, *k, **kw):
	kw['includes'] = Utils.to_list(kw.get('includes', []))
	kw['includes'] += [ctx.top_dir]
	ctx.objects(env=ctx.all_envs['cross'].derive(), *k, **kw)

@conf
def userprogram(ctx, *k, **kw):
	linkflags = '-u_start -nostartfiles -static'
	if 'baseaddr' in kw:
		linkflags = linkflags + ' -Wl,-Ttext -Wl,%s' % kw['baseaddr']
		del kw['baseaddr']
	ctx.crossprogram(use='system shared', linkflags=linkflags, *k, **kw)

@conf
def initfs(ctx, *k, **kw):
	inputs = []
	for file in Utils.to_list(kw['files']):
		tgen = ctx.get_tgen_by_name(file)
		tgen.post()
		inputs += [tgen.link_task.outputs[0]]

	mkinitfs = ctx.get_tgen_by_name('mkinitfs')
	mkinitfs.post()
	tmp = ctx.path.find_or_declare('InitFsData.tmp')
	data = ctx.path.find_or_declare('InitFsData.o')
	ctx(name='mkinitfs', rule='%s -o ${TGT} ${SRC}' % mkinitfs.link_task.outputs[0].bldpath(), source=inputs, target=tmp, env=ctx.all_envs['cross'].derive(), *k, **kw)
	ctx(name='pkginitfs', rule='"${OBJCOPY}" -I binary -O elf32-littlearm -B arm --rename-section .data=.initfs ${SRC} ${TGT}', source=tmp, target=data, env=ctx.all_envs['cross'].derive(), *k, **kw)
	attach = ctx.get_tgen_by_name(kw['attach'])
	attach.source += [data]

def build(ctx):
	ctx.add_group('tools')
	ctx.recurse('tools/mkinitfs')

	ctx.add_group('libs')
	ctx.recurse('lib')

	ctx.add_group('progs')
	ctx.recurse('progs')

	ctx.add_group('kernel')
	ctx.recurse('kernel')
	ctx.initfs(files='init console name clientA clientB echo shell hello crash log', attach='kernel')
