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
import java.util.*;

/**
 * This class is used for representing the inheritance tree during code
 * generation. You will need to fill in some of its methods and
 * potentially extend it in other useful ways.
 */
class CgenClassTable extends SymbolTable {

    /**
     * All classes in the program, represented as CgenNode
     */
    private Vector nds; /* nodes */

    /**
     * This is the stream to which assembly instructions are output
     */
    private PrintStream str;

    public HashMap<String, CgenNode> clsMp; /* 用于记录类的一些信息! */
    //public HashMap<String, HashMap<String, Integer>> dispTab;
    public DispTab dispTab; /* 我们需要构建一个dispTab */
    public AttrTab attrTab;
    //public HashMap<String, HashMap<String, Integer>> attrTab; /* 此外,我们还需要记录一个属性的偏移量的表 */
    public HashMap<String, Integer> argMp; /* 用于记录函数的参数列表 */
    public Stack<String> vitualFrame; /* 这个玩意用于模拟运行中的帧 */
    public HashMap<String, Integer> objTab;

    private int stringclasstag;
    private int intclasstag;
    private int boolclasstag;

    // The following methods emit code for constants and global
    // declarations.

    /**
     * Emits code to start the .data segment and to
     * declare the global names.
     */

    private void codeGlobalData() {
        // The following global names must be defined first.
        str.print("\t.data\n" + CgenSupport.ALIGN); // .data       .align 2
        str.println(CgenSupport.GLOBAL + CgenSupport.CLASSNAMETAB); //
        str.print(CgenSupport.GLOBAL);
        CgenSupport.emitProtObjRef(TreeConstants.Main, str);
        str.println("");
        str.print(CgenSupport.GLOBAL);
        CgenSupport.emitProtObjRef(TreeConstants.Int, str);
        str.println("");
        str.print(CgenSupport.GLOBAL);
        CgenSupport.emitProtObjRef(TreeConstants.Str, str);
        str.println("");
        str.print(CgenSupport.GLOBAL);
        BoolConst.falsebool.codeRef(str);
        str.println("");
        str.print(CgenSupport.GLOBAL);
        BoolConst.truebool.codeRef(str);
        str.println("");
        str.println(CgenSupport.GLOBAL + CgenSupport.INTTAG);
        str.println(CgenSupport.GLOBAL + CgenSupport.BOOLTAG);
        str.println(CgenSupport.GLOBAL + CgenSupport.STRINGTAG);

        // We also need to know the tag of the Int, String, and Bool classes
        // during code generation.

        str.println(CgenSupport.INTTAG + CgenSupport.LABEL // LABEL居然是":\n"
                + CgenSupport.WORD + intclasstag);
        str.println(CgenSupport.BOOLTAG + CgenSupport.LABEL
                + CgenSupport.WORD + boolclasstag);
        str.println(CgenSupport.STRINGTAG + CgenSupport.LABEL
                + CgenSupport.WORD + stringclasstag);

    }

    /**
     * Emits code to start the .text segment and to
     * declare the global names.
     */
    private void codeGlobalText() {
        /* 这里可是真正的代码区 */
        str.println(CgenSupport.GLOBAL + CgenSupport.HEAP_START);
        str.print(CgenSupport.HEAP_START + CgenSupport.LABEL);
        str.println(CgenSupport.WORD + 0);
        str.println("\t.text");
        str.print(CgenSupport.GLOBAL);
        CgenSupport.emitInitRef(TreeConstants.Main, str);
        str.println("");
        str.print(CgenSupport.GLOBAL);
        CgenSupport.emitInitRef(TreeConstants.Int, str);
        str.println("");
        str.print(CgenSupport.GLOBAL);
        CgenSupport.emitInitRef(TreeConstants.Str, str);
        str.println("");
        str.print(CgenSupport.GLOBAL);
        CgenSupport.emitInitRef(TreeConstants.Bool, str);
        str.println("");
        str.print(CgenSupport.GLOBAL);
        CgenSupport.emitMethodRef(TreeConstants.Main, TreeConstants.main_meth, str);
        str.println("");
    }

    /**
     * Emits code definitions for boolean constants.
     */
    private void codeBools(int classtag) {
        BoolConst.falsebool.codeDef(classtag, str);
        BoolConst.truebool.codeDef(classtag, str);
    }

    /**
     * Generates GC choice constants (pointers to GC functions)
     */
    private void codeSelectGc() {
        str.println(CgenSupport.GLOBAL + "_MemMgr_INITIALIZER");
        str.println("_MemMgr_INITIALIZER:"); // 这也是一个标号,是吧!
        str.println(CgenSupport.WORD
                + CgenSupport.gcInitNames[Flags.cgen_Memmgr]);

        str.println(CgenSupport.GLOBAL + "_MemMgr_COLLECTOR");
        str.println("_MemMgr_COLLECTOR:");
        str.println(CgenSupport.WORD
                + CgenSupport.gcCollectNames[Flags.cgen_Memmgr]);

        str.println(CgenSupport.GLOBAL + "_MemMgr_TEST");
        str.println("_MemMgr_TEST:");
        str.println(CgenSupport.WORD
                + ((Flags.cgen_Memmgr_Test == Flags.GC_TEST) ? "1" : "0"));
    }

    /**
     * Emits code to reserve space for and initialize all of the
     * constants.  Class names should have been added to the string
     * table (in the supplied code, is is done during the construction
     * of the inheritance graph), and code for emitting string constants
     * as a side effect adds the string's length to the integer table.
     * The constants are emmitted by running through the stringtable and
     * inttable and producing code for each entry.
     */
    private void codeConstants() {
        // Add constants that are required by the code generator.
        AbstractTable.stringtable.addString("");
        AbstractTable.inttable.addString("0");

        AbstractTable.stringtable.codeStringTable(stringclasstag, str);
        AbstractTable.inttable.codeStringTable(intclasstag, str);
        codeBools(boolclasstag);
    }

    /**
     * Creates data structures representing basic Cool classes (Object,
     * IO, Int, Bool, String).  Please note: as is this method does not
     * do anything useful; you will need to edit it to make if do what
     * you want.
     */
    private void installBasicClasses() {
        AbstractSymbol filename
                = AbstractTable.stringtable.addString("<basic class>");

        // A few special class names are installed in the lookup table
        // but not the class list.  Thus, these classes exist, but are
        // not part of the inheritance hierarchy.  No_class serves as
        // the parent of Object and the other special classes.
        // SELF_TYPE is the self class; it cannot be redefined or
        // inherited.  prim_slot is a class known to the code generator.

        addId(TreeConstants.No_class, // CgenClassTable继承自SymbolTable,其拥有一个hashmap的程源变量tbl,是吧!
                new CgenNode(new class_(0,
                        TreeConstants.No_class,
                        TreeConstants.No_class,
                        new Features(0),
                        filename),
                        CgenNode.Basic, this));

        addId(TreeConstants.SELF_TYPE,
                new CgenNode(new class_(0,
                        TreeConstants.SELF_TYPE,
                        TreeConstants.No_class,
                        new Features(0),
                        filename),
                        CgenNode.Basic, this)); // self_type居然也是基本的类

        addId(TreeConstants.prim_slot,
                new CgenNode(new class_(0,
                        TreeConstants.prim_slot,  // 这个玩意究竟是干什么的?
                        TreeConstants.No_class,
                        new Features(0),
                        filename),
                        CgenNode.Basic, this)); // 这是加入tbl折本hashtable之中吧,貌似用处不是很大啊!

        // The Object class has no parent class. Its methods are
        //        cool_abort() : Object    aborts the program
        //        type_name() : Str        returns a string representation
        //                                 of class name
        //        copy() : SELF_TYPE       returns a copy of the object

        class_ Object_class =
                new class_(0,
                        TreeConstants.Object_,
                        TreeConstants.No_class,
                        new Features(0)
                                .appendElement(new method(0,
                                        TreeConstants.cool_abort,
                                        new Formals(0),
                                        TreeConstants.Object_,
                                        new no_expr(0)))
                                .appendElement(new method(0,
                                        TreeConstants.type_name,
                                        new Formals(0),
                                        TreeConstants.Str,
                                        new no_expr(0)))
                                .appendElement(new method(0,
                                        TreeConstants.copy,
                                        new Formals(0),
                                        TreeConstants.SELF_TYPE,
                                        new no_expr(0))),
                        filename);
        // 居然调用installClass函数,有意思
        installClass(new CgenNode(Object_class, CgenNode.Basic, this));

        // The IO class inherits from Object. Its methods are
        //        out_string(Str) : SELF_TYPE  writes a string to the output
        //        out_int(Int) : SELF_TYPE      "    an int    "  "     "
        //        in_string() : Str            reads a string from the input
        //        in_int() : Int                "   an int     "  "     "

        class_ IO_class =
                new class_(0,
                        TreeConstants.IO,
                        TreeConstants.Object_,
                        new Features(0)
                                .appendElement(new method(0,
                                        TreeConstants.out_string,
                                        new Formals(0)
                                                .appendElement(new formal(0,
                                                        TreeConstants.arg,
                                                        TreeConstants.Str)),
                                        TreeConstants.SELF_TYPE,
                                        new no_expr(0)))
                                .appendElement(new method(0,
                                        TreeConstants.out_int,
                                        new Formals(0)
                                                .appendElement(new formal(0,
                                                        TreeConstants.arg,
                                                        TreeConstants.Int)),
                                        TreeConstants.SELF_TYPE,
                                        new no_expr(0)))
                                .appendElement(new method(0,
                                        TreeConstants.in_string,
                                        new Formals(0),
                                        TreeConstants.Str,
                                        new no_expr(0)))
                                .appendElement(new method(0,
                                        TreeConstants.in_int,
                                        new Formals(0),
                                        TreeConstants.Int,
                                        new no_expr(0))),
                        filename);

        installClass(new CgenNode(IO_class, CgenNode.Basic, this));

        // The Int class has no methods and only a single attribute, the
        // "val" for the integer.

        class_ Int_class =
                new class_(0,
                        TreeConstants.Int,
                        TreeConstants.Object_,
                        new Features(0)
                                .appendElement(new attr(0,
                                        TreeConstants.val,
                                        TreeConstants.prim_slot,
                                        new no_expr(0))),
                        filename);

        installClass(new CgenNode(Int_class, CgenNode.Basic, this));

        // Bool also has only the "val" slot.
        class_ Bool_class =
                new class_(0,
                        TreeConstants.Bool,
                        TreeConstants.Object_,
                        new Features(0)
                                .appendElement(new attr(0,
                                        TreeConstants.val,
                                        TreeConstants.prim_slot,
                                        new no_expr(0))),
                        filename);

        installClass(new CgenNode(Bool_class, CgenNode.Basic, this));

        // The class Str has a number of slots and operations:
        //       val                              the length of the string
        //       str_field                        the string itself
        //       length() : Int                   returns length of the string
        //       concat(arg: Str) : Str           performs string concatenation
        //       substr(arg: Int, arg2: Int): Str substring selection

        class_ Str_class =
                new class_(0,
                        TreeConstants.Str,
                        TreeConstants.Object_,
                        new Features(0)
                                .appendElement(new attr(0,
                                        TreeConstants.val,
                                        TreeConstants.Int,
                                        new no_expr(0)))
                                .appendElement(new attr(0,
                                        TreeConstants.str_field,
                                        TreeConstants.prim_slot,
                                        new no_expr(0)))
                                .appendElement(new method(0,
                                        TreeConstants.length,
                                        new Formals(0),
                                        TreeConstants.Int,
                                        new no_expr(0)))
                                .appendElement(new method(0,
                                        TreeConstants.concat,
                                        new Formals(0)
                                                .appendElement(new formal(0,
                                                        TreeConstants.arg,
                                                        TreeConstants.Str)),
                                        TreeConstants.Str,
                                        new no_expr(0)))
                                .appendElement(new method(0,
                                        TreeConstants.substr,
                                        new Formals(0)
                                                .appendElement(new formal(0,
                                                        TreeConstants.arg,
                                                        TreeConstants.Int))
                                                .appendElement(new formal(0,
                                                        TreeConstants.arg2,
                                                        TreeConstants.Int)),
                                        TreeConstants.Str,
                                        new no_expr(0))),
                        filename);

        installClass(new CgenNode(Str_class, CgenNode.Basic, this));
    }

    // The following creates an inheritance graph from
    // a list of classes.  The graph is implemented as
    // a tree of `CgenNode', and class names are placed
    // in the base class symbol table.

    private void installClass(CgenNode nd) {
        AbstractSymbol name = nd.getName(); // 得到类的名称,因为CgenNode实际上是继承自class_ 嘛
        if (probe(name) != null) return; // cgentable 继承自symboltable,probe方法主要是在tbl这个hashmap中寻找某个元素
        // 如果已经存在了,就不用再加入了,否则的话,就加入
        nds.addElement(nd); // 还有一个Vector的成员,专门用于记录CgenNode,是吧! 为什么要加入两遍呢?
        addId(name, nd); // 类名 <---> CgenNode
    }

    private void installClasses(Classes cs) {
        for (Enumeration e = cs.getElements(); e.hasMoreElements(); ) {
            installClass(new CgenNode((Class_) e.nextElement(),
                    CgenNode.NotBasic, this));
        }
    }

    private void buildInheritanceTree() { // 构建继承树
        for (Enumeration e = nds.elements(); e.hasMoreElements(); ) {
            setRelations((CgenNode) e.nextElement());
        }
    }

    private void setRelations(CgenNode nd) { // 来设置关系
        CgenNode parent = (CgenNode) probe(nd.getParent());
        nd.setParentNd(parent);
        parent.addChild(nd);
    }

    /**
     * Constructs a new class table and invokes the code generator
     */
    public CgenClassTable(Classes cls, PrintStream str) {
        nds = new Vector();

        this.str = str;

        stringclasstag = 4 /* Change to your String class tag here */;
        intclasstag = 2 /* Change to your Int class tag here */;
        boolclasstag = 3/* Change to your Bool class tag here */;
        argMp = new HashMap<>();
        vitualFrame = new Stack<>();
        enterScope(); // 进入一个Scope(),这和语义分析的时候是一样一样的.
        if (Flags.cgen_debug) System.out.println("Building CgenClassTable");

        installBasicClasses(); // 首先翻译一些简单的基本的类的信息
        installClasses(cls);
        buildInheritanceTree(); // 居然还有构建继承树

        code(); // 好吧,真的开始构建了!

        exitScope();
    }

    /**
     * This method is the meat of the code generator.  It is to be
     * filled in programming assignment 5
     */
    public void code() {
        if (Flags.cgen_debug) System.out.println("coding global data");
        codeGlobalData(); // 构建全局的数据区

        if (Flags.cgen_debug) System.out.println("choosing gc");
        codeSelectGc(); // 构建GC?这是啥玩意

        if (Flags.cgen_debug) System.out.println("coding constants");
        codeConstants(); // 构建常量

        // 接下来构建所谓的class_nameTab吧!
        if (Flags.cgen_debug) System.out.println("coding class name table");
        codeClassNameTab();

        if (Flags.cgen_debug) System.out.println("coding class Object table");
        codeClsObjTab();

        if (Flags.cgen_debug) System.out.println("coding dispatch tables");
        codeDispatchTab();

        if (Flags.cgen_debug) System.out.println("coding port Object");
        codePrototype();

        if (Flags.cgen_debug) System.out.println("coding global text");
        codeGlobalText();

        if (Flags.cgen_debug) System.out.println("coding object initializer");
        codeObjInit();

        if (Flags.cgen_debug) System.out.println("coding class method");
        codeClass();
    }

    private attr getHighOrderAttr() {
        /* 用于寻找更高的优先级的属性 */
        return null;
    }

    private attr getNextInitAttr(CgenNode cg, HashSet<attr> initializedAttr) {
        /* 用于获取下一个应该初始化的属性 */
        boolean flag = false;
        return null;
    }

    private ArrayList<attr> getInitOrder(CgenNode cg, HashSet<attr> needInitAttr) {
        /* needInitAttr代表的是需要被初始化的变量 */
        ArrayList<attr> order = new ArrayList<>();
        HashSet<attr> initializedAttr = new HashSet<>(); /* 用于存储已经初始化完成了排序的属性 */
        Enumeration fen = cg.features.getElements();
        return null;
    }

    public void codeObjInit() {
        /* 什么叫做初始化?很简单,实际上要初始化的东西只有attr */
        Enumeration en = nds.elements();
        while (en.hasMoreElements()) {
            CgenNode cg = (CgenNode) en.nextElement();
            str.print(cg.getName().str + "_init" + CgenSupport.LABEL);
            /* 接下来确实是要开始生成代码了!我暂时也不知道应该怎样生成,不如先放在这里吧! */
            CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, -12, str);
            CgenSupport.emitStore(CgenSupport.FP, 3, CgenSupport.SP, str);
            CgenSupport.emitStore(CgenSupport.SELF, 2, CgenSupport.SP, str);
            CgenSupport.emitStore(CgenSupport.RA, 1, CgenSupport.SP, str);
            CgenSupport.emitAddiu(CgenSupport.FP, CgenSupport.SP, 4, str);
            str.println("# 现在开始将self放入s0之中,以便以后使用!");
            CgenSupport.emitMove(CgenSupport.SELF, CgenSupport.ACC, str);
            /* 接下来就是如果有初始化的动作的话,这里要继续修改! */
            String parentName = cg.getParentNd().name.str;
            /* 先初始化父类的部分 */
            str.println("# 先将父类的部分初始化!");
            if (!parentName.equals("_no_class"))
                CgenSupport.emitJal(parentName + CgenSupport.CLASSINIT_SUFFIX, str);

            /**
             * 这个玩意其实讲究一个初始化的次序,原则上是让依赖程度低的先初始化.因此,我们要构建这样的一个
             * 初始化的次序表.
             */

            int counter = 1;
            while (counter != 0) {
                Enumeration fen = cg.features.getElements();
                while (fen.hasMoreElements()) {
                    Feature ft = (Feature) fen.nextElement();
                    if (ft instanceof attr) {
                        // 要求是赋值操作,并且要有初始化的表达式.
                        attr atr = (attr) ft;
                        Expression expr = atr.init; /* 初始化的表达式 */
                        if (!(expr instanceof no_expr)) { /*必须要存在init才行*/
                            /**
                             * 这里非常有必要来提一下一个Object的layout,前面的一个WORD表示class tag,第二个WORD表示Object的大小,
                             * 第三个WORD里面存放了这个Object的函数指针.从第四个字节开始才是真正的attr的地址.
                             */
                            str.println("# 别告诉我没有这一段,very important!");
                            expr.code(str, cg, this);
                            int offset = 3 + attrTab.getClsAttrOffset(cg.getName().getString(), atr.name.str);
                            // 算了,我这里就不进行垃圾收集了.
                            System.out.println(offset);
                            CgenSupport.emitStore(CgenSupport.ACC, offset, CgenSupport.SELF, str); /* 好吧,我看见了,果然是赋值操作 */
                        }
                    }
                }
                counter--;
            }

            CgenSupport.emitMove(CgenSupport.ACC, CgenSupport.SELF, str);
            CgenSupport.emitLoad(CgenSupport.FP, 3, CgenSupport.SP, str);
            CgenSupport.emitLoad(CgenSupport.SELF, 2, CgenSupport.SP, str);
            CgenSupport.emitLoad(CgenSupport.RA, 1, CgenSupport.SP, str);
            CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, 12, str);
            CgenSupport.emitReturn(str);
        }
    }

    private Stack<CgenNode> findParent(CgenNode cg) {
        Stack<CgenNode> stk = new Stack<>();
        if (cg == null) return stk;
        else stk.push(cg);

        cg = cg.getParentNd();
        while (true) {
            if (cg == null) break;
            else {
                stk.push(cg);
                cg = cg.getParentNd();
            }
        }
        return stk;
    }

    private int codeAttr(CgenNode cg) {
        int counter = 0;
        Enumeration en = cg.getFeatures().getElements();
        //System.out.println(cg.getName());
        while (en.hasMoreElements()) {
            Feature ft = (Feature) en.nextElement();
            if (ft instanceof attr) {
                //str.println(CgenSupport.WORD + "0"); // 未初始化的值都放0吧!
                //System.out.println("hi");
                counter++;
            }
        }
        return counter;
    }

    public void codePrototype() {
        /** 好的,这里要编码真正的Object了!
         * 我有一个实现的想法,其实很简单,那就是从这个节点出发,找到它所有的父节点,然后遍历
         * 直到找到所有的属性.聪明吧!
         * 这里需要两个属性,一个是类的标号 class tag,另外一个是这个类的大小.
         * 类的大小涉及到了attr的个数
         */
        Enumeration en = nds.elements(); /* nds */
        attrTab = new AttrTab();
        int tag = 0;
        while (en.hasMoreElements()) {
            CgenNode cg = (CgenNode) en.nextElement();
            str.println(CgenSupport.WORD + "-1");
            str.print(cg.getName().str + "_protObj" + CgenSupport.LABEL);
            str.println(CgenSupport.WORD + tag); // 好吧,得到class tag
            cg.setClassTag(tag++);
            Stack<CgenNode> stk = findParent(cg); // 找到其所所有的父节点,包括自己
            int counter = 0;
            while (!stk.empty()) {
                CgenNode ncg = stk.pop();
                counter += codeAttr(ncg);
            }
            str.println(CgenSupport.WORD + (counter + 3)); /* 大小 */
            str.println(CgenSupport.WORD + cg.getName() + "_dispTab");
            codePrototypeObj(cg, cg, attrTab);

        }

    }

    private void codePrototypeObj(CgenNode cls, CgenNode cg, AttrTab attrTab) {
        if (!cg.getName().equals(TreeConstants.Object_)) { // 不断往上面递归
            codePrototypeObj(cls, cg.getParentNd(), attrTab);
        }

        Enumeration en = cg.features.getElements();
        while (en.hasMoreElements()) {
            Feature ft = (Feature) en.nextElement();
            if (ft instanceof attr) {
                attr atr = (attr) ft;
                if (atr.type_decl.equals(TreeConstants.Int)) {
                    IntSymbol is = (IntSymbol) AbstractTable.inttable.lookup("0");
                    str.print(CgenSupport.WORD);
                    is.codeRef(str);
                    str.println("");
                } else if (atr.type_decl.equals(TreeConstants.Str)) {
                    StringSymbol ss = (StringSymbol) AbstractTable.stringtable.lookup("");
                    str.print(CgenSupport.WORD);
                    ss.codeRef(str);
                    str.println("");
                } else if (atr.type_decl.equals(TreeConstants.Bool)) {
                    str.print(CgenSupport.WORD);
                    BoolConst.falsebool.codeRef(str);
                    str.println("");
                } else {
                    str.println(CgenSupport.WORD + "0");
                }
                attrTab.addClsAttr(cls.getName().str, atr);
                //mp.put(cg.getName().getString() + "." + atr.name.str, mp.size()); /* 记录下来偏移量 */
            }
        }

    }

    public void codeClsObjTab() {
        objTab = new HashMap<>(); /* 这里需要记录下偏移的地址,方便以后查询 */
        int counter = 0;
        str.print("class_objTab" + CgenSupport.LABEL);
        Enumeration en = nds.elements();
        while (en.hasMoreElements()) {
            CgenNode cg = (CgenNode) en.nextElement();
            String protObj = cg.getName().str + CgenSupport.PROTOBJ_SUFFIX; /* xxx__protObj */
            objTab.put(protObj, counter++);  /* 记录下类的原型的偏移地址 */
            str.println(CgenSupport.WORD + protObj);

            String init = cg.getName().str + CgenSupport.CLASSINIT_SUFFIX; /* xxx__init */
            objTab.put(init, counter++); /* 记录下初始化函数的偏移地址 */
            str.println(CgenSupport.WORD + init);
        }
    }

    public void codeDispatchTab() {
        /** 接下来要构建所谓的Dispatch表,接下来就要在类中寻找相应的方法了.
         * 好吧,最简单的方法其实是从某个类开始遍历.
         */
        dispTab = new DispTab();
        Enumeration en = nds.elements();
        while (en.hasMoreElements()) {
            CgenNode cg = (CgenNode) en.nextElement();
            str.print(cg.getName().str + CgenSupport.DISPTAB_SUFFIX + CgenSupport.LABEL);
            getClsDispatchTab(cg, cg, dispTab);
            Vector<Pair> pairs = dispTab.getTab(cg.name.str);
            for (int i = 0; i < pairs.size(); ++i) {
                str.println(CgenSupport.WORD + pairs.get(i).toString());
            }
        }
        str.println(CgenSupport.WORD + "-1");
    }

    private void getClsDispatchTab(CgenNode self, CgenNode base, DispTab dispTab) {
        if (!base.getName().equals(TreeConstants.Object_)) {
            getClsDispatchTab(self, base.getParentNd(), dispTab);
        }
        Enumeration en = base.getFeatures().getElements();
        while (en.hasMoreElements()) {
            Feature ft = (Feature) en.nextElement();
            if (ft instanceof method) {
                method mtd = (method) ft;
                System.out.print("getClsDispatchTab --> selfName :" + self.name.str);
                System.out.print(" baseName :" + base.name.str);
                System.out.print(" mtdName :" + mtd.name.str);
                System.out.println("");
                dispTab.addClsMethod(self.name.str, base.name.str, mtd.name.str);
            }
        }
    }

    public void codeClass() {
        Enumeration en = nds.elements();
        while (en.hasMoreElements()) {
            CgenNode cg = (CgenNode) en.nextElement();
            if (!cg.basic())  /* We only generate code for not bascic class! */
                codeClassMethod(cg);
        }
    }

    private void codeClassMethod(CgenNode cg) {
        Features fts = cg.getFeatures();
        Enumeration en = fts.getElements();


        while (en.hasMoreElements()) {
            Feature ft = (Feature) en.nextElement();
            if (ft instanceof method) { // 我们只需要编码方法即可.
                method mtd = (method) ft;
                str.print(cg.name.str + CgenSupport.METHOD_SEP + mtd.name.str + CgenSupport.LABEL);
                CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, -12, str);
                CgenSupport.emitStore(CgenSupport.FP, 3, CgenSupport.SP, str);      /* 总是要记录下FP,SELF($s0),RA三个指针原来的值 */
                CgenSupport.emitStore(CgenSupport.SELF, 2, CgenSupport.SP, str);
                CgenSupport.emitStore(CgenSupport.RA, 1, CgenSupport.SP, str);
                CgenSupport.emitAddiu(CgenSupport.FP, CgenSupport.SP, 4, str);
                CgenSupport.emitMove(CgenSupport.SELF, CgenSupport.ACC, str);
                Enumeration aen = mtd.formals.getElements();
                while (aen.hasMoreElements()) {
                    formal fm = (formal) aen.nextElement();
                    argMp.put(fm.name.str, argMp.size()); /* 记录下偏移量 */
                }

                mtd.expr.code(str, cg, this); // 或许,我还要添加另外的一个参数
                argMp.clear(); /* 一定要记得,每一次做完之后都要清空,才能完事 */
                CgenSupport.emitLoad(CgenSupport.FP, 3, CgenSupport.SP, str);
                CgenSupport.emitLoad(CgenSupport.SELF, 2, CgenSupport.SP, str);
                CgenSupport.emitLoad(CgenSupport.RA, 1, CgenSupport.SP, str);
                /* 调用这个函数时会压入numOfFormals个参数,因此函数返回时要回退这么多个参数 */
                CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, mtd.formals.getLength() * 4 + 12, str);
                CgenSupport.emitReturn(str);
            }
        }

    }

    public void codeClassNameTab() {
        /** 这里其实是要构建所谓的classNameTable,所以要搜集class的名称,并且要将名称对应到之前已经产生了的string中
         *  既然所有的类都已经集中在了CgenClassTable之中,所以我们只需要遍历即可.
         */
        str.println("class_nameTab:");

        // 好吧,让我们来看一看nds里面究竟有写什么?
        Enumeration en = nds.elements();
        while (en.hasMoreElements()) {
            CgenNode cg = (CgenNode) en.nextElement();
            StringSymbol ss = (StringSymbol) AbstractTable.stringtable.lookup(cg.getName().str);
            str.print(CgenSupport.WORD);
            ss.codeRef(str);
            str.println("");
            //System.out.println(cg.getName().str);
        }
        //str.println(CgenSupport.WORD + "-1");
    }

    /**
     * Gets the root of the inheritance tree
     */
    public CgenNode root() {
        return (CgenNode) probe(TreeConstants.Object_);
    }
}

    
