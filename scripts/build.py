#!/usr/bin/env python
 
from glob import glob
import json

import os
import itertools
import hashlib
import subprocess
import sys

PATH = 'C:/devkitPro/devkitARM/bin'
PREFIX = '/arm-none-eabi-'
AS = (PATH + PREFIX + 'as')
CC = (PATH + PREFIX + 'gcc')
LD = (PATH + PREFIX + 'ld')
OBJCOPY = (PATH + PREFIX + 'objcopy')
ROOT = './'
SRC = ROOT+'src/'
BUILT_GRAPHICS = './src/built_graphics'
BUILD = './build'
ASFLAGS = ['-mthumb']
LDFLAGS = ['-z','muldefs','-T', 'linker.ld', '-T', 'BPRE.ld', '-r']
CFLAGS= ['-Isrc/include', '-mthumb', '-mno-thumb-interwork', '-mcpu=arm7tdmi',
         '-fno-inline', '-mlong-calls', '-march=armv4t', '-fno-builtin', '-Wall', '-O2', '-std=c11']
#rifai: hardcoded palette e ow che usa 0 (trasparenza) 1 (chiaro) 2(scuro) e 3(contorno)
GRITFLAGS=['-gB4',      #4bpp
           '-pn16',     #pal is 16 colours
           '-pu16',      #pal is u16
           '-pS',
           '-aw16',    #area width is 16
           '-gTa349a4', #rgb(163, 73, 164) (#0xa349a4) is trasparency
           '-ftc']      #file_type: c
 
def run_command(cmd):
    try:
        subprocess.check_output(cmd)
    except subprocess.CalledProcessError as e:
        print(e.output.decode(), file=sys.stderr)
        sys.exit(1)
 
def make_output_file(filename):
    '''Return hash of filename to use as object filename'''
    m = hashlib.md5()
    m.update(filename.encode())
    return os.path.join(BUILD, m.hexdigest() + '.o')
 
def process_assembly(in_file):
    '''Assemble'''
    out_file = make_output_file(in_file)
    cmd = [AS] + ASFLAGS + ['-c', in_file, '-o', out_file]
    run_command(cmd)
    return out_file
 
def process_c(in_file):
    '''Compile C'''
    out_file = make_output_file(in_file)
    cmd = [CC] + CFLAGS + ['-c', in_file, '-o', out_file]
    run_command(cmd)
    return out_file
 
from string import Template

def process_img(in_file):
    '''Compile IMGs'''
    print(in_file)
    # imgs are first converted to .c/.h files, then built like the rest of the source code
    out_file = os.path.join(os.path.dirname(in_file), '..', 'built_graphics', os.path.basename(in_file))

    print('Running Grit on '+os.path.abspath(out_file)+ ", basefile: "+in_file)
    cmd = ['grit', in_file, '-o', out_file] + GRITFLAGS
    run_command(cmd)


    return out_file
 
def link(objects):
    '''Link objects into one binary'''
    linked = 'build/linked.o'
    cmd = [LD] + LDFLAGS + ['-o', linked] + list(objects)
    run_command(cmd)
    return linked
 
def objcopy(binary):
    cmd = [OBJCOPY, '-O', 'binary', binary, 'build/output.bin']
    run_command(cmd)
 
def run_glob(globstr, fn):
    '''Glob recursively and run the processor function on each file in result'''
    files = glob(os.path.join(ROOT, globstr), recursive=True)
    return map(fn, files)

def clear_folder(foldername):
    '''Empty folder of files'''
    files = glob(foldername+'/*')
    for f in files:
        os.remove(f)

 
from string import Template

def main():
    if len(sys.argv) < 2:
        print("No custom address for the compiled code has been set.")
        print("To add a custom address simply put it after the python executable name, like so: python scripts/build.py 0x08800000.")
        print("Aborting...")
        return
    
    
    with open('config.json', 'rb') as f:
        config = json.load(f)

    customAddr = config["freespace_start"]
    print("Inserting code at "+customAddr)
 
    # Create output directory
    try:
        os.makedirs(BUILD)
        os.makedirs(BUILT_GRAPHICS)
    except FileExistsError:
        pass

    clear_folder(BUILD)
    clear_folder(BUILT_GRAPHICS)
    
    img_globs = {
        './src/**/*.png'
    }

    for globstr in img_globs:
        files = glob(os.path.join(ROOT, globstr), recursive=True)
        cmd = ['grit']
        for file in files:
            cmd.append(os.path.normpath(os.path.join("../../", file)))
        cmd.extend(GRITFLAGS)
        #print(' '.join(cmd))
        proc = subprocess.Popen(' '.join(cmd), cwd=r'src\graphics')
        proc.wait()
    
    compiled_img_globs = {
        './src/graphics/**/*.c',
        './src/graphics/**/*.h'
    }
    for globstr in compiled_img_globs:
        files = glob(os.path.join(ROOT, globstr), recursive=True)
        for file in files:
            new_file=os.path.join(os.path.dirname(file), '..', 'built_graphics', os.path.basename(file))
            #print("Relocating "+file+" to "+new_file)
            os.rename(file, new_file)

    

    globs = {
        '**/*.c': process_c
    }
 
    # Gather source files and process them
    objects = itertools.starmap(run_glob, globs.items())
 
    # Link and extract raw binary
    linked = link(itertools.chain.from_iterable(objects))
    objcopy(linked)
	
    ow_equs = ['-equ', 'shared_pal_tag', config["shared_ow_pal_tag"]] #array of equ definitions to be passed to armips

    # Read from picture tbl entry to convert from picture_no to ow offset
    with open('BPRE0.gba', 'rb') as f:
        for i in config["pictures_no_by_type"]: 
            #for each pictureNo, find the entry in the table that maps it to the ow sprite data
            picture_tbl_entry = int(config["picture_no_tbl_addr"],16) + int(config["picture_no_tbl_sizeof"]) * int(config["pictures_no_by_type"][i]) 
            f.seek(picture_tbl_entry, 0)
            ow_sprite_tbl_addr = f.read(4)
            ow_sprite_tbl_addr = int.from_bytes(ow_sprite_tbl_addr, byteorder='little')

            ow_sprite_addr = ow_sprite_tbl_addr + 0x1C # sprite bitmap is at offset 0x1C
            ow_sprite_pal_tag = ow_sprite_tbl_addr + 0x2 # sprite pal tag is at offset 0x2

            f.seek(ow_sprite_addr - 0x8000000, 0) #remove 0x8000000 indication of ROM
            ow_sprite = f.read(4)
            ow_sprite = int.from_bytes(ow_sprite, byteorder='little')

            ow_equs.append('-equ')
            ow_equs.append('gray_'+i+"_tile_addr")
            ow_equs.append(hex(ow_sprite))
            ow_equs.append('-equ')
            ow_equs.append('gray_'+i+"_pal_tag_addr")
            ow_equs.append(hex(ow_sprite_pal_tag))

	#ARMPIS
    cmd = ['armips', './src/main.s','-sym','symbols.txt', '-equ', 'freespace', customAddr, '-equ', 'shared_pal_addr', config["shared_ow_pal_addr"]] + ow_equs
    #sprint(' '.join(cmd))
    run_command(cmd)


    callasm_entrypoint_line = None 
    #compile sample script with data from symbols.txt
    with open('./symbols.txt', 'r') as file:
        for line in file:
            if 'callasm_entrypoint' in line:
                callasm_entrypoint_line = line.strip()

    if callasm_entrypoint_line:
        callasm_entrypoint_address = hex(int(callasm_entrypoint_line.split(" ")[0], 16)+1)
        print("found callasm_entrypoint address: "+callasm_entrypoint_address)
        placeholder = '{{CALLASM_ENTRYPOINT_ADDR}}'
        sample_script_file = './sample_script.txt'
        compiled_script_file = './compiled_script.txt'
        with open(sample_script_file, 'r') as file:
            # Read all lines from the input file
            lines = file.readlines()

        with open(compiled_script_file, 'w') as file:
            for line in lines:
                modified_line = line.replace(placeholder, callasm_entrypoint_address)
                file.write(modified_line)
    else:
        raise Exception("unable to find callasm_entrypoint in symbols.txt file: no compiled_script can be created at this time. Check your compilation.")

        
    


    print("DONE")
 
if __name__ == '__main__':
    main()
