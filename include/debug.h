/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: include/debug.h
 * 
 * Copyright (c) 2012 Clemens Bernhard Geyer <clemens.geyer@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef SIM_DBG

#define perror() {printf("ERROR in line %d!\n", __LINE__);}
#define pdbg(varname, value) {\
	printf("DBG: Value of %s: %p in line %d.\n", \
		varname, (void *) value, __LINE__); \
}
#else

#define perror() 
#define pdbg(varname, value)

#endif /* SIM_DBG */

#endif /* __DEBUG_H__ */
