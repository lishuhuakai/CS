/*
Copyright (c) 2000 The Regents of the University of California.
All rights reserved.

Permission to use, copy, modify, and distribute this software for any
purpose, without fee, and without written agreement is hereby granted,
provided that the above copyright notice and the following two
paragraphs appear in all copies of this software.

IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

// This is a project skeleton file

import java.io.PrintStream;
import java.util.Vector;
import java.util.Enumeration;

class CgenNode extends class_ { /* node */
    /**
     * The parent of this node in the inheritance tree
     */
    private CgenNode parent;

    /**
     * The children of this node in the inheritance tree
     */
    private Vector children;

    /**
     * Indicates a basic class
     */
    final static int Basic = 0;

    /**
     * 好吧,这个变量是我自行添加的,并没有什么含义.
     */
    private int classTag;

    /**
     * Indicates a class that came from a Cool program
     */
    final static int NotBasic = 1;

    /**
     * Does this node correspond to a basic class?
     */
    private int basic_status; /* 这个玩意究竟是用来干什么的呢? */

    /**
     * Constructs a new CgenNode to represent class "c".
     *
     * @param c            the class
     * @param basic_status is this class basic or not
     * @param table        the class table
     */
    CgenNode(Class_ c, int basic_status, CgenClassTable table) { /* 它们将这个table传递进来是几个意思? */
        super(0, c.getName(), c.getParent(), c.getFeatures(), c.getFilename());
        this.parent = null;
        this.children = new Vector();
        this.basic_status = basic_status;
        /* 不能这么玩啊,迟早要出问题的. */
        this.classTag = 0;

        AbstractTable.stringtable.addString(name.getString()); /* 添加一个新的类名 */
    }
    public void setClassTag(int tag) {
        this.classTag = tag;
    }

    void addChild(CgenNode child) {
        children.addElement(child);
    }
    public int getClassTag() {
        return classTag;
    }
    /**
     * Gets the children of this class
     *
     * @return the children
     */
    Enumeration getChildren() {
        return children.elements();
    }

    /**
     * Sets the parent of this class.
     *
     * @param parent the parent
     */
    void setParentNd(CgenNode parent) { /* 这其实和assign3中的继承图非常类似! */
        if (this.parent != null) {
            Utilities.fatalError("parent already set in CgenNode.setParent()");
        }
        if (parent == null) {
            Utilities.fatalError("null parent in CgenNode.setParent()");
        }
        this.parent = parent;
    }


    /**
     * Gets the parent of this class
     *
     * @return the parent
     */
    CgenNode getParentNd() {
        return parent;
    }

    /**
     * Returns true is this is a basic class.
     *
     * @return true or false
     */
    boolean basic() {
        return basic_status == Basic;
    }
}
    

    
