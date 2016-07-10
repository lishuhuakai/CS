# Wes Weimer
#
# This allows students with minimal knowledge to produce pretty pictures of
# HTML webpages.
#
import sys
import subprocess
import os

# If you want the output filenames to be different (e.g., based on
# environment variables), just change them here.
output_latex_filename = "./student"

# If you want to put the static images elsewhere, just change this.
image_directory = "images/"     # make "" for current directory

# The example image output requires these packages:
#
#       pdflatex        (aka "pdftex")
#       imagemagick     (for "convert")
#       ghostscript     (called by "convert")

outfile = None
logfile = None

import base64
import json
import sys


def word(x):
        global outfile
        for i in range(len(x)):
          if x[i] == '_':
            outfile.write("\_")
          elif x[i] != '\\':
            outfile.write(x[i])
        outfile.write (" ")

def warning(x):
        global outfile
        outfile.write("{\\color{red}{\\bf{" + x + "}}}")

closetags = []

def pushclosing(x):
        global closetags
        closetags = [x] + closetags
def begintag(tag,args):
        global outfile
        global logfile
        tag = tag.lower()
        # need "IMG"
        logfile.write("TAG + " + tag + "\n")
        if tag == "a":
                if "href" in args:
                        target = args["href"]
                        outfile.write("\\href{" + target + "}{\\underline{")
                        pushclosing("}}")
                else:
                        warning("invalid 'a' tag: no 'href' argument")
                        pushclosing("")
        elif tag == "img":
                if "src" in args:
                        target = args["src"]
                        filename = image_directory + target
                        if os.path.isfile(filename):
                          if "height" in args and "width" in args:
                            h = args["height"]
                            w = args["width"]
                            outfile.write("\\includegraphics[height=" + h + "px, width=" + w + "px]{" + filename + "}")
                            pushclosing("")
                          else:
                            outfile.write("\\includegraphics{" + filename + "}")
                            pushclosing("")
                        else:
                          warning("'img' " + target + " not found (predefined local images only, sorry)")
                          pushclosing("")
                else:
                        warning("invalid 'img' tag: no 'src' argument")
                        pushclosing("")
        elif tag == "b" or tag == "strong":
                outfile.write("\\textbf{")
                pushclosing("}")
        elif tag == "ul":
                outfile.write("\\begin{itemize}")
                pushclosing("\\end{itemize}")
        elif tag == "ol":
                outfile.write("\\begin{enumerate}")
                pushclosing("\\end{enumerate}")
        elif tag == "li":
                outfile.write("\\item{")
                pushclosing("}")
        elif tag == "big":
                outfile.write("{\\Large ")
                pushclosing("}")
        elif tag == "tt" or tag == "code":
                outfile.write("{\\tt ")
                pushclosing("}")
        elif tag == "small":
                outfile.write("{\\footnotesize ")
                pushclosing("}")
        elif tag == "i" or tag == "em":
                outfile.write("\\emph{")
                pushclosing("}")
        elif tag == "hr":
                outfile.write("{\\begin{center} \\line(1,0){400} \\end{center}}")
                pushclosing("")
        elif tag == "h1":
                outfile.write("\\section*{")
                pushclosing("}")
        elif tag == "h2":
                outfile.write("\\subsection*{")
                pushclosing("}")
        elif tag == "h3":
                outfile.write("\\subsubsection*{")
                pushclosing("}")
        elif tag == "p" or tag == "br":
                outfile.write("\n~\n\n\\noindent ")
                pushclosing("\n")
        else:
                pushclosing("")

def endtag():
        global outfile
        global logfile
        global closetags
        if closetags == []:
                raise IndexError
        tag = closetags[0]
        closetags = closetags[1:]
        logfile.write("TAG -\n")
        outfile.write(tag)

def initialize():
        global outfile
        global logfile
        global output_latex_filename
        outfile = open(output_latex_filename + ".tex",'w+')
        logfile = open(output_latex_filename + ".taglog",'w+')
        outfile.write("""
\\documentclass{article}
\\usepackage{fullpage}
\\usepackage{hyperref}
\\hypersetup{
  colorlinks,%
    citecolor=blue,%
    filecolor=blue,%
    linkcolor=blue,%
    urlcolor=blue
}
\\usepackage{graphicx}
\\usepackage{color}
\\usepackage{url}
\\usepackage{geometry}
\\pagestyle{empty}
\\begin{document}
\\mag 1440
""")

def finalize():
        global outfile
        global logfile
        logfile.close()
        outfile.write("""
\\end{document}
""")
        #print "Writing TEX Output: " + output_latex_filename + ".tex"
        outfile.close()
        #print "Rendering PDF Graphics: " + output_latex_filename + ".pdf"
        cmd = "pdflatex " + output_latex_filename + ".tex > /dev/null < /dev/null"
        subprocess.call(cmd,shell=True)
        #print "Rendering PNG Graphics: " + output_latex_filename + ".png"
        cmd = "convert " + output_latex_filename + ".pdf " + \
                output_latex_filename + ".png"
        subprocess.call(cmd,shell=True)
