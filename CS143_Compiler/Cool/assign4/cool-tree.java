// -*- mode: java -*- 
//
// file: cool-tree.m4
//
// This file defines the AST
//
//////////////////////////////////////////////////////////


import java.io.PrintStream;
import java.util.Enumeration;
import java.util.Vector;


/**
 * Defines simple phylum Program
 */
abstract class Program extends TreeNode {
    protected Program(int lineNumber) {
        super(lineNumber);
    }

    public abstract void dump_with_types(PrintStream out, int n);

    public abstract void semant();

    public abstract void cgen(PrintStream s);

}


/**
 * Defines simple phylum Class_
 */
abstract class Class_ extends TreeNode {
    protected Class_(int lineNumber) {
        super(lineNumber);
    }

    public abstract void dump_with_types(PrintStream out, int n);

    public abstract AbstractSymbol getName();

    public abstract AbstractSymbol getParent();

    public abstract AbstractSymbol getFilename();

    public abstract Features getFeatures();

}


/**
 * Defines list phylum Classes
 * <p>
 * See <a href="ListNode.html">ListNode</a> for full documentation.
 */
class Classes extends ListNode {
    public final static Class elementClass = Class_.class;

    /**
     * Returns class of this lists's elements
     */
    public Class getElementClass() {
        return elementClass;
    }

    protected Classes(int lineNumber, Vector elements) {
        super(lineNumber, elements);
    }

    /**
     * Creates an empty "Classes" list
     */
    public Classes(int lineNumber) {
        super(lineNumber);
    }

    /**
     * Appends "Class_" element to this list
     */
    public Classes appendElement(TreeNode elem) {
        addElement(elem);
        return this;
    }

    public TreeNode copy() {
        return new Classes(lineNumber, copyElements());
    }
}


/**
 * Defines simple phylum Feature
 */
abstract class Feature extends TreeNode {
    protected Feature(int lineNumber) {
        super(lineNumber);
    }

    public abstract void dump_with_types(PrintStream out, int n);

}


/**
 * Defines list phylum Features
 * <p>
 * See <a href="ListNode.html">ListNode</a> for full documentation.
 */
class Features extends ListNode {
    public final static Class elementClass = Feature.class;

    /**
     * Returns class of this lists's elements
     */
    public Class getElementClass() {
        return elementClass;
    }

    protected Features(int lineNumber, Vector elements) {
        super(lineNumber, elements);
    }

    /**
     * Creates an empty "Features" list
     */
    public Features(int lineNumber) {
        super(lineNumber);
    }

    /**
     * Appends "Feature" element to this list
     */
    public Features appendElement(TreeNode elem) {
        addElement(elem);
        return this;
    }

    public TreeNode copy() {
        return new Features(lineNumber, copyElements());
    }
}


/**
 * Defines simple phylum Formal
 */
abstract class Formal extends TreeNode {
    protected Formal(int lineNumber) {
        super(lineNumber);
    }

    public abstract void dump_with_types(PrintStream out, int n);

}


/**
 * Defines list phylum Formals
 * <p>
 * See <a href="ListNode.html">ListNode</a> for full documentation.
 */
class Formals extends ListNode {
    public final static Class elementClass = Formal.class;

    /**
     * Returns class of this lists's elements
     */
    public Class getElementClass() {
        return elementClass;
    }

    protected Formals(int lineNumber, Vector elements) {
        super(lineNumber, elements);
    }

    /**
     * Creates an empty "Formals" list
     */
    public Formals(int lineNumber) {
        super(lineNumber);
    }

    /**
     * Appends "Formal" element to this list
     */
    public Formals appendElement(TreeNode elem) {
        addElement(elem);
        return this;
    }

    public TreeNode copy() {
        return new Formals(lineNumber, copyElements());
    }
}


/**
 * Defines simple phylum Expression
 */
abstract class Expression extends TreeNode {
    protected Expression(int lineNumber) {
        super(lineNumber);
    }

    private AbstractSymbol type = null;

    public AbstractSymbol get_type() {
        return type;
    }

    public void cgen(PrintStream s) {

    }

    public Expression set_type(AbstractSymbol s) {
        type = s;
        return this;
    }

    public abstract void dump_with_types(PrintStream out, int n);

    public void dump_type(PrintStream out, int n) {
        if (type != null) {
            out.println(Utilities.pad(n) + ": " + type.getString());
        } else {
            out.println(Utilities.pad(n) + ": _no_type");
        }
    }

    public abstract void code(PrintStream s, class_ cls, CgenClassTable ctb);

}


/**
 * Defines list phylum Expressions
 * <p>
 * See <a href="ListNode.html">ListNode</a> for full documentation.
 */
class Expressions extends ListNode {
    public final static Class elementClass = Expression.class;

    /**
     * Returns class of this lists's elements
     */
    public Class getElementClass() {
        return elementClass;
    }

    protected Expressions(int lineNumber, Vector elements) {
        super(lineNumber, elements);
    }

    /**
     * Creates an empty "Expressions" list
     */
    public Expressions(int lineNumber) {
        super(lineNumber);
    }

    /**
     * Appends "Expression" element to this list
     */
    public Expressions appendElement(TreeNode elem) {
        addElement(elem);
        return this;
    }

    public TreeNode copy() {
        return new Expressions(lineNumber, copyElements());
    }
}


/**
 * Defines simple phylum Case
 */
abstract class Case extends TreeNode {
    protected Case(int lineNumber) {
        super(lineNumber);
    }

    public abstract void dump_with_types(PrintStream out, int n);

}


/**
 * Defines list phylum Cases
 * <p>
 * See <a href="ListNode.html">ListNode</a> for full documentation.
 */
class Cases extends ListNode {
    public final static Class elementClass = Case.class;

    /**
     * Returns class of this lists's elements
     */
    public Class getElementClass() {
        return elementClass;
    }

    protected Cases(int lineNumber, Vector elements) {
        super(lineNumber, elements);
    }

    /**
     * Creates an empty "Cases" list
     */
    public Cases(int lineNumber) {
        super(lineNumber);
    }

    /**
     * Appends "Case" element to this list
     */
    public Cases appendElement(TreeNode elem) {
        addElement(elem);
        return this;
    }

    public TreeNode copy() {
        return new Cases(lineNumber, copyElements());
    }
}


/**
 * Defines AST constructor 'program'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class program extends Program {
    public Classes classes;

    /**
     * Creates "program" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for classes
     */
    public program(int lineNumber, Classes a1) {
        super(lineNumber);
        classes = a1;
    }

    public TreeNode copy() {
        return new program(lineNumber, (Classes) classes.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "program\n");
        classes.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_program");
        for (Enumeration e = classes.getElements(); e.hasMoreElements(); ) {
            ((Class_) e.nextElement()).dump_with_types(out, n + 1);
        }
    }

    /**
     * This method is the entry point to the semantic checker.  You will
     * need to complete it in programming assignment 4.
     * <p>
     * Your checker should do the following two things:
     * <ol>
     * <li>Check that the program is semantically correct
     * <li>Decorate the abstract syntax tree with type information
     * by setting the type field in each Expression node.
     * (see tree.h)
     * </ol>
     * <p>
     * You are free to first do (1) and make sure you catch all semantic
     * errors. Part (2) can be done in a second stage when you want
     * to test the complete compiler.
     */
    public void semant() {
    /* ClassTable constructor may do some semantic analysis */
        ClassTable classTable = new ClassTable(classes);

	/* some semantic analysis code may go here */

        if (classTable.errors()) {
            System.err.println("Compilation halted due to static semantic errors.");
            System.exit(1);
        }
    }

    /**
     * This method is the entry point to the code generator.  All of the work
     * of the code generator takes place within CgenClassTable constructor.
     *
     * @param s the output stream
     * @see CgenClassTable
     */
    public void cgen(PrintStream s) {
        // spim wants comments to start with '#'
        s.print("# start of generated code\n");
        // 首先构建一个CgenClassTable,是吧!
        CgenClassTable codegen_classtable = new CgenClassTable(classes, s);

        s.print("\n# end of generated code\n");
    }

}


/**
 * Defines AST constructor 'class_'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class class_ extends Class_ {
    public AbstractSymbol name; // 这个表示类的名字
    public AbstractSymbol parent;
    public Features features;
    public AbstractSymbol filename;

    /**
     * Creates "class_" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     * @param a2         initial value for parent
     * @param a3         initial value for features
     * @param a4         initial value for filename
     */
    public class_(int lineNumber, AbstractSymbol a1, AbstractSymbol a2, Features a3, AbstractSymbol a4) {
        super(lineNumber);
        name = a1;
        parent = a2;
        features = a3;
        filename = a4;
    }

    public TreeNode copy() {
        return new class_(lineNumber, copy_AbstractSymbol(name), copy_AbstractSymbol(parent), (Features) features.copy(), copy_AbstractSymbol(filename));
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "class_\n");
        dump_AbstractSymbol(out, n + 2, name);
        dump_AbstractSymbol(out, n + 2, parent);
        features.dump(out, n + 2);
        dump_AbstractSymbol(out, n + 2, filename);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_class");
        dump_AbstractSymbol(out, n + 2, name);
        dump_AbstractSymbol(out, n + 2, parent);
        out.print(Utilities.pad(n + 2) + "\"");
        Utilities.printEscapedString(out, filename.getString());
        out.println("\"\n" + Utilities.pad(n + 2) + "(");
        for (Enumeration e = features.getElements(); e.hasMoreElements(); ) {
            ((Feature) e.nextElement()).dump_with_types(out, n + 2);
        }
        out.println(Utilities.pad(n + 2) + ")");
    }

    public AbstractSymbol getName() {
        return name;
    }

    public AbstractSymbol getParent() {
        return parent;
    }

    public AbstractSymbol getFilename() {
        return filename;
    }

    public Features getFeatures() {
        return features;
    }

}


/**
 * Defines AST constructor 'method'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class method extends Feature {
    public AbstractSymbol name;
    public Formals formals;
    public AbstractSymbol return_type;
    public Expression expr;

    /**
     * Creates "method" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     * @param a2         initial value for formals
     * @param a3         initial value for return_type
     * @param a4         initial value for expr
     */
    public method(int lineNumber, AbstractSymbol a1, Formals a2, AbstractSymbol a3, Expression a4) {
        super(lineNumber);
        name = a1;
        formals = a2;
        return_type = a3;
        expr = a4;
    }

    public TreeNode copy() {
        return new method(lineNumber, copy_AbstractSymbol(name), (Formals) formals.copy(), copy_AbstractSymbol(return_type), (Expression) expr.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "method\n");
        dump_AbstractSymbol(out, n + 2, name);
        formals.dump(out, n + 2);
        dump_AbstractSymbol(out, n + 2, return_type);
        expr.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_method");
        dump_AbstractSymbol(out, n + 2, name);
        for (Enumeration e = formals.getElements(); e.hasMoreElements(); ) {
            ((Formal) e.nextElement()).dump_with_types(out, n + 2);
        }
        dump_AbstractSymbol(out, n + 2, return_type);
        expr.dump_with_types(out, n + 2);
    }

}


/**
 * Defines AST constructor 'attr'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class attr extends Feature {
    public AbstractSymbol name;
    public AbstractSymbol type_decl;
    public Expression init;

    /**
     * Creates "attr" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     * @param a2         initial value for type_decl
     * @param a3         initial value for init
     */
    public attr(int lineNumber, AbstractSymbol a1, AbstractSymbol a2, Expression a3) {
        super(lineNumber);
        name = a1;
        type_decl = a2;
        init = a3;
    }

    public TreeNode copy() {
        return new attr(lineNumber, copy_AbstractSymbol(name), copy_AbstractSymbol(type_decl), (Expression) init.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "attr\n");
        dump_AbstractSymbol(out, n + 2, name);
        dump_AbstractSymbol(out, n + 2, type_decl);
        init.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_attr");
        dump_AbstractSymbol(out, n + 2, name);
        dump_AbstractSymbol(out, n + 2, type_decl);
        init.dump_with_types(out, n + 2);
    }

}


/**
 * Defines AST constructor 'formal'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class formal extends Formal {
    public AbstractSymbol name;
    public AbstractSymbol type_decl;

    /**
     * Creates "formal" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     * @param a2         initial value for type_decl
     */
    public formal(int lineNumber, AbstractSymbol a1, AbstractSymbol a2) {
        super(lineNumber);
        name = a1;
        type_decl = a2;
    }

    public TreeNode copy() {
        return new formal(lineNumber, copy_AbstractSymbol(name), copy_AbstractSymbol(type_decl));
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "formal\n");
        dump_AbstractSymbol(out, n + 2, name);
        dump_AbstractSymbol(out, n + 2, type_decl);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_formal");
        dump_AbstractSymbol(out, n + 2, name);
        dump_AbstractSymbol(out, n + 2, type_decl);
    }

}


/**
 * Defines AST constructor 'branch'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class branch extends Case {
    public AbstractSymbol name;
    public AbstractSymbol type_decl;
    public Expression expr;

    /**
     * Creates "branch" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     * @param a2         initial value for type_decl
     * @param a3         initial value for expr
     */
    public branch(int lineNumber, AbstractSymbol a1, AbstractSymbol a2, Expression a3) {
        super(lineNumber);
        name = a1;
        type_decl = a2;
        expr = a3;
    }

    public TreeNode copy() {
        return new branch(lineNumber, copy_AbstractSymbol(name), copy_AbstractSymbol(type_decl), (Expression) expr.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "branch\n");
        dump_AbstractSymbol(out, n + 2, name);
        dump_AbstractSymbol(out, n + 2, type_decl);
        expr.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_branch");
        dump_AbstractSymbol(out, n + 2, name);
        dump_AbstractSymbol(out, n + 2, type_decl);
        expr.dump_with_types(out, n + 2);
    }

}


/**
 * Defines AST constructor 'assign'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class assign extends Expression {
    public AbstractSymbol name;
    public Expression expr;

    /**
     * Creates "assign" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     * @param a2         initial value for expr
     */
    public assign(int lineNumber, AbstractSymbol a1, Expression a2) {
        super(lineNumber);
        name = a1;
        expr = a2;
    }

    public TreeNode copy() {
        return new assign(lineNumber, copy_AbstractSymbol(name), (Expression) expr.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "assign\n");
        dump_AbstractSymbol(out, n + 2, name);
        expr.dump(out, n + 2);
    }

    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_assign");
        dump_AbstractSymbol(out, n + 2, name);
        expr.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        int offset = 0;
        expr.code(s, cls, ctb);
        System.out.println("assign --> left : " + name.str + " right: " + expr);
        /* 什么才是所谓的赋值运算呢?那就是将变量的值真正改掉 */
        /**
         * 所谓的赋值操作,要修改的变量,第一个是在let表达式中,然后是某个类实体的attr
         */
        for (int i = ctb.vitualFrame.size() - 1; i >= 0; --i) {
            if (ctb.vitualFrame.get(i).equals(name.str)) {
                offset = ctb.vitualFrame.size() - i;
                System.out.println(offset);
                CgenSupport.emitStore(CgenSupport.ACC, offset, CgenSupport.SP, s);
                return;
            }
        }

        Object obj = ctb.argMp.get(name.str);
        if (obj != null) {
            System.out.println("assign --> ------2------");
            offset = (int) obj + 3;
            CgenSupport.emitStore(CgenSupport.ACC, offset, CgenSupport.FP, s);
        } else {

            /**
             * 这种事情,始终是要面对的,迟做不如早做.这里之所以找不到的原因其实非常简单,那就是
             */
            System.out.println(cls.name.str + " " + name.str);
            offset = ctb.attrTab.getClsAttrOffset(cls.name.str, name.str) + 3;
            System.out.println("assign --> offset : " + offset);
            //offset = ctb.attrTab.get(cls.name.str).get(name.str) + 3;
            CgenSupport.emitStore(CgenSupport.ACC, offset, CgenSupport.SELF, s);
        }
    }
}


/**
 * Defines AST constructor 'static_dispatch'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class static_dispatch extends Expression {
    public Expression expr;
    public AbstractSymbol type_name;
    public AbstractSymbol name;
    public Expressions actual;
    private int num_args;

    public int numOfArgs() {
        if (num_args >= 0) return num_args;
        int counter = 0;
        Enumeration en = actual.getElements();
        while (en.hasMoreElements()) {
            en.nextElement();
            ++counter;
        }
        num_args = counter;
        return num_args;
    }

    /**
     * Creates "static_dispatch" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for expr
     * @param a2         initial value for type_name
     * @param a3         initial value for name
     * @param a4         initial value for actual
     */
    public static_dispatch(int lineNumber, Expression a1, AbstractSymbol a2, AbstractSymbol a3, Expressions a4) {
        super(lineNumber);
        expr = a1;
        type_name = a2;
        name = a3;
        actual = a4;
        num_args = -1;
    }


    public TreeNode copy() {
        return new static_dispatch(lineNumber, (Expression) expr.copy(), copy_AbstractSymbol(type_name), copy_AbstractSymbol(name), (Expressions) actual.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "static_dispatch\n");
        expr.dump(out, n + 2);
        dump_AbstractSymbol(out, n + 2, type_name);
        dump_AbstractSymbol(out, n + 2, name);
        actual.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_static_dispatch");
        expr.dump_with_types(out, n + 2);
        dump_AbstractSymbol(out, n + 2, type_name);
        dump_AbstractSymbol(out, n + 2, name);
        out.println(Utilities.pad(n + 2) + "(");
        for (Enumeration e = actual.getElements(); e.hasMoreElements(); ) {
            ((Expression) e.nextElement()).dump_with_types(out, n + 2);
        }
        out.println(Utilities.pad(n + 2) + ")");
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        /**
         * 不管是怎样的函数调用,要做的第一件事情,就是将参数压栈
         */
        int numOfArgs = actual.getLength(); /* 获得实际参数的个数 */
        System.out.println("static_dispatch --> " + name.str); /* 输出要调用的函数的名字*/

        /* 先将实际参数压栈 */
        Enumeration en = actual.getElements();
        s.println("# 我们要开始将实际参数压栈了!");
        while (en.hasMoreElements()) {
            Expression expr = (Expression) en.nextElement();
            expr.code(s, cls, ctb); /* 逐个计算实际参数,计算的结果在$a0之中 */
            CgenSupport.emitPush(CgenSupport.ACC, s); /* 将实际的参数压栈 */
            ctb.vitualFrame.push("**NO**");
        }
        s.println("# 现在开始计算调用者的地址,放在a0之中!");
        expr.code(s, cls, ctb); /* 获得表达式的值,结果也在$a0之中 */

        int label = CgenSupport.getLabel(); /* 这个时候需要跳转到另外一个label去寻找dispatchTab了! */
        CgenSupport.emitBne(CgenSupport.ACC, CgenSupport.ZERO, label, s);
        CgenSupport.emitLoadAddress(CgenSupport.ACC, "str_const0", s); /* 载入文件名对应的string Object*/
        CgenSupport.emitLoadImm(CgenSupport.T1, 1, s);
        CgenSupport.emitJal("_dispatch_abort", s);

        /* 下面我们要将 */
        String dispLabel = type_name.str + CgenSupport.DISPTAB_SUFFIX;

        s.println("# 重新构建一个label!");
        CgenSupport.emitLabelDef(label, s); /* 构建一个新的label */

        /** 将self的dispatchTab的地址放入了$t1,我们必须要知道,self地址往后移动8个单位,就到了dispatchTab
         * 接下来要做的事情其实也非常明显,既然dispatchTab的地址到了$t1中,我们只需要从dispatchTab中寻找我们要
         * 调用的函数的地址即可.怎样来寻找呢?之前在输出的时候,我们就应该记录下来偏移量.方便后面的查询.
         */
        s.println("# 得到对象的函数指针的地址,放入t1之中!");
        CgenSupport.emitLoadAddress(CgenSupport.T1, dispLabel, s);

        String callerType = type_name.str;
        String mtdName = name.str;

        System.out.println("static_dispatch --> " + "调用者所属的类: " + callerType + "调用的方法: " + name.str);

        int offset = ctb.dispTab.getClsMtdOffset(callerType, mtdName);
        s.println("# 得到调用函数所在的地址,放入t1之中!");
        CgenSupport.emitLoad(CgenSupport.T1, offset, CgenSupport.T1, s); /* 将函数指针载入$t1中*/
        s.println("# 跳转到函数所在地址,开始执行函数!");
        CgenSupport.emitJalr(CgenSupport.T1, s); /* 跳转到函数处开始执行 */
        for (int i = 0; i < numOfArgs; ++i) {
            ctb.vitualFrame.pop();
        }

    }

}


/**
 * Defines AST constructor 'dispatch'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class dispatch extends Expression {
    public Expression expr;
    public AbstractSymbol name;
    public Expressions actual;

    /**
     * Creates "dispatch" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for expr
     * @param a2         initial value for name
     * @param a3         initial value for actual
     */
    public dispatch(int lineNumber, Expression a1, AbstractSymbol a2, Expressions a3) {
        super(lineNumber);
        expr = a1;
        name = a2;
        actual = a3;
    }

    public TreeNode copy() {
        return new dispatch(lineNumber, (Expression) expr.copy(), copy_AbstractSymbol(name), (Expressions) actual.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "dispatch\n");
        expr.dump(out, n + 2);
        dump_AbstractSymbol(out, n + 2, name);
        actual.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_dispatch");
        expr.dump_with_types(out, n + 2);
        dump_AbstractSymbol(out, n + 2, name);
        out.println(Utilities.pad(n + 2) + "(");
        for (Enumeration e = actual.getElements(); e.hasMoreElements(); ) {
            ((Expression) e.nextElement()).dump_with_types(out, n + 2);
        }
        out.println(Utilities.pad(n + 2) + ")");
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        int numOfArgs = actual.getLength(); /* 获得实际参数的个数 */
        System.out.println("dispatch --> " + name.str); /* 输出要调用的函数的名字*/

        /* 先将实际参数压栈 */
        Enumeration en = actual.getElements();
        s.println("# 我们要开始将实际参数压栈了!");
        while (en.hasMoreElements()) {
            Expression expr = (Expression) en.nextElement();
            expr.code(s, cls, ctb); /* 逐个计算实际参数,计算的结果在$a0之中 */
            CgenSupport.emitPush(CgenSupport.ACC, s); /* 将实际的参数压栈 */
            ctb.vitualFrame.push("**NO**");
        }
        s.println("# 现在开始计算调用者的地址,放在a0之中!");
        expr.code(s, cls, ctb); /* 获得表达式的值,结果也在$a0之中 */

        int label = CgenSupport.getLabel(); /* 这个时候需要跳转到另外一个label去寻找dispatchTab了! */

        /* 如果调用者不为空,则跳转至label出执行 */
        s.println("# 如果调用者的地址不为空的话,开始调用函数" + name.str + "啦!");
        CgenSupport.emitBne(CgenSupport.ACC, CgenSupport.ZERO, label, s);
        CgenSupport.emitLoadAddress(CgenSupport.ACC, "str_const0", s); /* 载入文件名对应的string Object*/
        CgenSupport.emitLoadImm(CgenSupport.T1, 1, s);
        CgenSupport.emitJal("_dispatch_abort", s);
        AbstractSymbol sb = expr.get_type();

        /**
         * 这里,我需要说明一下,根据书中的约定,一般而言$a0中存放的是SELF的地址,也就是自己这个object的首地址啦
         */
        s.println("# 重新构建一个label!");
        CgenSupport.emitLabelDef(label, s); /* 构建一个新的label */
        s.println("# 得到对象的函数指针的地址,放入t1之中!");
        CgenSupport.emitLoad(CgenSupport.T1, 2, CgenSupport.ACC, s);
        /** 将self的dispatchTab的地址放入了$t1,我们必须要知道,self地址往后移动8个单位,就到了dispatchTab
         * 接下来要做的事情其实也非常明显,既然dispatchTab的地址到了$t1中,我们只需要从dispatchTab中寻找我们要
         * 调用的函数的地址即可.怎样来寻找呢?之前在输出的时候,我们就应该记录下来偏移量.方便后面的查询.
         */
        String callerType = expr.get_type().str;
        String mtdName = name.str;
        if (expr.get_type().equals(TreeConstants.SELF_TYPE)) {
            /* 如果是SELF_TYPE类型,那么调用者就是self所指代的类 */
            callerType = cls.name.str;
        }

        System.out.println("dispatch --> " + "调用者所属的类: " + expr.get_type().str + "调用的方法: " + name.str);

        int offset = ctb.dispTab.getClsMtdOffset(callerType, mtdName);
        s.println("# 得到调用函数所在的地址,放入t1之中!");
        CgenSupport.emitLoad(CgenSupport.T1, offset, CgenSupport.T1, s); /* 将函数指针载入$t1中*/
        s.println("# 跳转到函数所在地址,开始执行函数!");
        CgenSupport.emitJalr(CgenSupport.T1, s); /* 跳转到函数处开始执行 */
        for (int i = 0; i < numOfArgs; ++i) {
            ctb.vitualFrame.pop();
        }
    }


}


/**
 * Defines AST constructor 'cond'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class cond extends Expression {
    public Expression pred;
    public Expression then_exp;
    public Expression else_exp;

    /**
     * Creates "cond" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for pred
     * @param a2         initial value for then_exp
     * @param a3         initial value for else_exp
     */
    public cond(int lineNumber, Expression a1, Expression a2, Expression a3) {
        super(lineNumber);
        pred = a1;
        then_exp = a2;
        else_exp = a3;
    }

    public TreeNode copy() {
        return new cond(lineNumber, (Expression) pred.copy(), (Expression) then_exp.copy(), (Expression) else_exp.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "cond\n");
        pred.dump(out, n + 2);
        then_exp.dump(out, n + 2);
        else_exp.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_cond");
        pred.dump_with_types(out, n + 2);
        then_exp.dump_with_types(out, n + 2);
        else_exp.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        s.println("# 欢迎进入cond-if表达式的编码境界!");
        pred.code(s, cls, ctb);
        /* bool类型其实和int类型是同一回事, 它们实际的值都在这个Object的第三个WORD处 */
        CgenSupport.emitLoad(CgenSupport.T1, 3, CgenSupport.ACC, s);
        int outCondLabel = CgenSupport.getLabel();
        int elseLable = CgenSupport.getLabel();
        CgenSupport.emitBeqz(CgenSupport.T1, elseLable, s);
        then_exp.code(s, cls, ctb);

        /**
         * 运行到了这里说明then已经运行完成了.我们要跳过else_branch,直接跳出if语句
         */
        s.print("\tb\t");
        CgenSupport.emitLabelRef(outCondLabel, s);
        s.println("");

        CgenSupport.emitLabelDef(elseLable, s); // 定义一个新的label
        else_exp.code(s, cls, ctb);

        CgenSupport.emitLabelDef(outCondLabel, s);
    }


}


/**
 * Defines AST constructor 'loop'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class loop extends Expression {
    public Expression pred;
    public Expression body;

    /**
     * Creates "loop" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for pred
     * @param a2         initial value for body
     */
    public loop(int lineNumber, Expression a1, Expression a2) {
        super(lineNumber);
        pred = a1;
        body = a2;
    }

    public TreeNode copy() {
        return new loop(lineNumber, (Expression) pred.copy(), (Expression) body.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "loop\n");
        pred.dump(out, n + 2);
        body.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_loop");
        pred.dump_with_types(out, n + 2);
        body.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        s.println("# 欢迎进入while循环的世界!");
        int trueLabel = CgenSupport.getLabel();
        int falseLabel = CgenSupport.getLabel();
        CgenSupport.emitLabelDef(trueLabel, s); /* 正确的话,往这边走 */
        pred.code(s, cls, ctb);
        s.println("# 取出pred计算的结果!");
        CgenSupport.emitLoad(CgenSupport.T1, 3, CgenSupport.ACC, s);
        CgenSupport.emitBeqz(CgenSupport.T1, falseLabel, s);
        body.code(s, cls, ctb);
        /* 跳转到truebranch处继续运行 */
        CgenSupport.emitBranch(trueLabel, s);
        CgenSupport.emitLabelDef(falseLabel, s); /* 如果测试不通过,就跳转到这里 */
        CgenSupport.emitMove(CgenSupport.ACC, CgenSupport.ZERO, s); /* 返回值为0 */
    }


}


/**
 * Defines AST constructor 'typcase'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class typcase extends Expression {
    public Expression expr;
    public Cases cases;

    /**
     * Creates "typcase" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for expr
     * @param a2         initial value for cases
     */
    public typcase(int lineNumber, Expression a1, Cases a2) {
        super(lineNumber);
        expr = a1;
        cases = a2;
    }

    public TreeNode copy() {
        return new typcase(lineNumber, (Expression) expr.copy(), (Cases) cases.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "typcase\n");
        expr.dump(out, n + 2);
        cases.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_typcase");
        expr.dump_with_types(out, n + 2);
        for (Enumeration e = cases.getElements(); e.hasMoreElements(); ) {
            ((Case) e.nextElement()).dump_with_types(out, n + 2);
        }
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        /**
         *  case语句最大的难点在于不在于case语句,在于多态.所以为了方便比较,我们的tag的选取必须有章可循,具体而言,
         *  那就是,祖宗的级别越高,那么它的tag的标号就应该越小.
         *  上面是一种解决的方法啦,另外一种的话,我们不是有了一个叫做继承图的东西吗?这可是好东西,
         */
        s.println("# 现在进行case语句的境界!");
        expr.code(s, cls, ctb);
        s.println("# 现在将结果保存到t3之中");
        CgenSupport.emitMove(CgenSupport.T3, CgenSupport.ACC, s);

        int label = CgenSupport.getLabel();
        s.println("# 现在要求结果不为空!否则的话,就要结束程序了!");
        CgenSupport.emitBne(CgenSupport.ACC, CgenSupport.ZERO, label, s);
        /* 载入文件名 */
        CgenSupport.emitLoadAddress(CgenSupport.ACC, "str_const0", s);
        CgenSupport.emitLoadImm(CgenSupport.T1, 1, s);
        CgenSupport.emitJal("_case_abort2", s);

        CgenSupport.emitLabelDef(label, s); /* 定义一个label */
        s.println("# 现在取得结果的类型,并将其保存在t4之中");
        CgenSupport.emitLoad(CgenSupport.T4, 0, CgenSupport.ACC, s);

        int outCaseBrc = CgenSupport.getLabel(); /* 这个标号用于跳出case语句 */
        Enumeration en = cases.getElements();
        while (en.hasMoreElements()) {
            /**
             *  得到一个分支,我觉得我们压根就没有必要编码出所有的branch,我们只需要符合条件的一个就可以了, 想法是非常好的,
             *  但是,结果你到运行的时候才知道,所以我们必须编码出所有的分支语句.我们可以采用流水式的编译.
             *  */
            branch brc = (branch) en.nextElement();
            CgenNode cg = (CgenNode) ctb.lookup(brc.type_decl); /* 查找对应的CgenNode */
            int clsTag = cg.getClassTag();
            //CgenSupport.emitBranch();
            CgenSupport.emitLoadImm(CgenSupport.T1, clsTag, s); /* 将类型的tag载入t1 */
            /* 如果标号不想等的话,进入下一轮比较 */
            label = CgenSupport.getLabel();
            CgenSupport.emitBne(CgenSupport.T1, CgenSupport.T4, label, s);
            /* 在计算结果之前,我们要将结果压栈 */
            CgenSupport.emitPush(CgenSupport.T3, s);

            ctb.vitualFrame.push(brc.name.str);
            brc.expr.code(s, cls, ctb);

            s.println("# 计算完成之后要出栈!");
            CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, 4, s);
            ctb.vitualFrame.pop();

            s.println("# 运行完成之后直接跳出case语句!");
            CgenSupport.emitB(outCaseBrc, s);
            CgenSupport.emitLabelDef(label, s); /* 定义一个label */
        }
        CgenSupport.emitJal("_case_abort", s);
        CgenSupport.emitLabelDef(outCaseBrc, s);
        s.println("# 现在取出case的条件计算后的值");
        CgenSupport.emitLoad(CgenSupport.T1, 0, CgenSupport.T3, s);
    }

}


/**
 * Defines AST constructor 'block'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class block extends Expression {
    public Expressions body;

    /**
     * Creates "block" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for body
     */
    public block(int lineNumber, Expressions a1) {
        super(lineNumber);
        body = a1;
    }

    public TreeNode copy() {
        return new block(lineNumber, (Expressions) body.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "block\n");
        body.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_block");
        for (Enumeration e = body.getElements(); e.hasMoreElements(); ) {
            ((Expression) e.nextElement()).dump_with_types(out, n + 2);
        }
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        /* 好吧,开始block语句的转化了! */
        Enumeration en = body.getElements();
        while (en.hasMoreElements()) {
            Expression expr = (Expression) en.nextElement();
            //System.out.println(expr);
            expr.code(s, cls, ctb); /* 对block语句中的每一个语句进行编码 */
        }
    }

}


/**
 * Defines AST constructor 'let'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class let extends Expression {
    public AbstractSymbol identifier;
    public AbstractSymbol type_decl;
    public Expression init;
    public Expression body;

    /**
     * Creates "let" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for identifier
     * @param a2         initial value for type_decl
     * @param a3         initial value for init
     * @param a4         initial value for body
     */
    public let(int lineNumber, AbstractSymbol a1, AbstractSymbol a2, Expression a3, Expression a4) {
        super(lineNumber);
        identifier = a1;
        type_decl = a2;
        init = a3;
        body = a4;
    }

    public TreeNode copy() {
        return new let(lineNumber, copy_AbstractSymbol(identifier), copy_AbstractSymbol(type_decl), (Expression) init.copy(), (Expression) body.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "let\n");
        dump_AbstractSymbol(out, n + 2, identifier);
        dump_AbstractSymbol(out, n + 2, type_decl);
        init.dump(out, n + 2);
        body.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_let");
        dump_AbstractSymbol(out, n + 2, identifier);
        dump_AbstractSymbol(out, n + 2, type_decl);
        init.dump_with_types(out, n + 2);
        body.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
         /* 加载默认的值,如果没有初始化动作的话! */
        if (init instanceof no_expr) {
            if (type_decl.equals(TreeConstants.Int)) {
                CgenSupport.emitLoadInt(CgenSupport.ACC,
                        ((IntSymbol) AbstractTable.inttable.lookup("0")), s);
            } else if (type_decl.equals(TreeConstants.Str)) {
                CgenSupport.emitLoadString(CgenSupport.ACC,
                        ((StringSymbol) AbstractTable.stringtable.lookup("")), s);
            } else if (type_decl.equals(TreeConstants.Bool)) {
                CgenSupport.emitLoadBool(CgenSupport.ACC, BoolConst.falsebool, s);
            } else {
                /* 居然载入一个立即数0 */
                CgenSupport.emitLoadImm(CgenSupport.ACC, 0, s);
            }
        } else {
            init.code(s, cls, ctb);
        }
        CgenSupport.emitPush(CgenSupport.ACC, s); /* 将结果压栈 */
        ctb.vitualFrame.push(identifier.str); /* 将变量压入栈中 */
        body.code(s, cls, ctb);
        ctb.vitualFrame.pop(); /* 出栈 */
        /* 执行完这个表达式之后,总要保证sp还停留在原来的位置*/
        CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, 4, s);
    }


}


/**
 * Defines AST constructor 'plus'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class plus extends Expression {
    public Expression e1;
    public Expression e2;

    /**
     * Creates "plus" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for e1
     * @param a2         initial value for e2
     */
    public plus(int lineNumber, Expression a1, Expression a2) {
        super(lineNumber);
        e1 = a1;
        e2 = a2;
    }

    public TreeNode copy() {
        return new plus(lineNumber, (Expression) e1.copy(), (Expression) e2.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "plus\n");
        e1.dump(out, n + 2);
        e2.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_plus");
        e1.dump_with_types(out, n + 2);
        e2.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        // 其实,我想到了一件非常有意思的事情,那就是,为什么我们不模拟一下运行时的栈呢?
        // 这样做起来的话,我想会简单很多啊.
        e1.code(s, cls, ctb);
        CgenSupport.emitPush(CgenSupport.ACC, s); // 将e1运算后的结果压栈
        ctb.vitualFrame.push("**No**"); /* 栈是要随时模拟的! */
        e2.code(s, cls, ctb); /* 计算后的结果放在$a0之中 */
        // 这么说,我们要另外地构建一个Int对象,要复制的对象的指针放在$a0之中,然后结果的指针也放在$a0之中
        CgenSupport.emitJal("Object.copy", s);
        CgenSupport.emitLoad(CgenSupport.T2, 3, CgenSupport.ACC, s); //  取出实际的整数值,在Int对象偏移量12的位置
        CgenSupport.emitLoad(CgenSupport.T1, 1, CgenSupport.SP, s); // get e1's result
        CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, 4, s); // 退栈
        ctb.vitualFrame.pop();

        CgenSupport.emitLoad(CgenSupport.T1, 3, CgenSupport.T1, s); // 取出实际的值
        CgenSupport.emitAdd(CgenSupport.T1, CgenSupport.T1, CgenSupport.T2, s);
        CgenSupport.emitStore(CgenSupport.T1, 3, CgenSupport.ACC, s);
        /* 我要说的一句是,结果确实是存放在$a0之中 */
    }


}


/**
 * Defines AST constructor 'sub'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class sub extends Expression {
    public Expression e1;
    public Expression e2;

    /**
     * Creates "sub" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for e1
     * @param a2         initial value for e2
     */
    public sub(int lineNumber, Expression a1, Expression a2) {
        super(lineNumber);
        e1 = a1;
        e2 = a2;
    }

    public TreeNode copy() {
        return new sub(lineNumber, (Expression) e1.copy(), (Expression) e2.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "sub\n");
        e1.dump(out, n + 2);
        e2.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_sub");
        e1.dump_with_types(out, n + 2);
        e2.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        // 这里的代码和前面的非常类似
        e1.code(s, cls, ctb);
        CgenSupport.emitPush(CgenSupport.ACC, s); // 将e1运算后的结果压栈
        ctb.vitualFrame.push("**No**"); /* 栈是要随时模拟的! */

        e2.code(s, cls, ctb); /* 计算后的结果放在$a0之中 */
        // 这么说,我们要另外地构建一个Int对象,要复制的对象的指针放在$a0之中,然后结果的指针也放在$a0之中
        CgenSupport.emitJal("Object.copy", s);
        CgenSupport.emitLoad(CgenSupport.T2, 3, CgenSupport.ACC, s); //  取出实际的整数值,在Int对象偏移量12的位置
        CgenSupport.emitLoad(CgenSupport.T1, 1, CgenSupport.SP, s); // get e1's result
        CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, 4, s); // 退栈
        ctb.vitualFrame.pop(); /* 一定要同时记得退栈 */

        CgenSupport.emitLoad(CgenSupport.T1, 3, CgenSupport.T1, s); // 取出实际的值
        CgenSupport.emitSub(CgenSupport.T1, CgenSupport.T1, CgenSupport.T2, s);
        CgenSupport.emitStore(CgenSupport.T1, 3, CgenSupport.ACC, s);
    }


}


/**
 * Defines AST constructor 'mul'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class mul extends Expression {
    public Expression e1;
    public Expression e2;

    /**
     * Creates "mul" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for e1
     * @param a2         initial value for e2
     */
    public mul(int lineNumber, Expression a1, Expression a2) {
        super(lineNumber);
        e1 = a1;
        e2 = a2;
    }

    public TreeNode copy() {
        return new mul(lineNumber, (Expression) e1.copy(), (Expression) e2.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "mul\n");
        e1.dump(out, n + 2);
        e2.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_mul");
        e1.dump_with_types(out, n + 2);
        e2.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        e1.code(s, cls, ctb);
        CgenSupport.emitPush(CgenSupport.ACC, s); // 将e1运算后的结果压栈
        ctb.vitualFrame.push("**No**"); /* 栈是要随时模拟的! */

        e2.code(s, cls, ctb); /* 计算后的结果放在$a0之中 */
        // 这么说,我们要另外地构建一个Int对象,要复制的对象的指针放在$a0之中,然后结果的指针也放在$a0之中
        CgenSupport.emitJal("Object.copy", s);
        CgenSupport.emitLoad(CgenSupport.T2, 3, CgenSupport.ACC, s); //  取出实际的整数值,在Int对象偏移量12的位置
        CgenSupport.emitLoad(CgenSupport.T1, 1, CgenSupport.SP, s); // get e1's result
        CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, 4, s); // 退栈
        ctb.vitualFrame.pop(); /* 一定要同时记得退栈 */

        CgenSupport.emitLoad(CgenSupport.T1, 3, CgenSupport.T1, s); // 取出实际的值
        CgenSupport.emitMul(CgenSupport.T1, CgenSupport.T1, CgenSupport.T2, s);
        CgenSupport.emitStore(CgenSupport.T1, 3, CgenSupport.ACC, s);
        /* 我要说的一句是,结果确实是存放在$a0之中 */
    }


}


/**
 * Defines AST constructor 'divide'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class divide extends Expression {
    public Expression e1;
    public Expression e2;

    /**
     * Creates "divide" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for e1
     * @param a2         initial value for e2
     */
    public divide(int lineNumber, Expression a1, Expression a2) {
        super(lineNumber);
        e1 = a1;
        e2 = a2;
    }

    public TreeNode copy() {
        return new divide(lineNumber, (Expression) e1.copy(), (Expression) e2.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "divide\n");
        e1.dump(out, n + 2);
        e2.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_divide");
        e1.dump_with_types(out, n + 2);
        e2.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        e1.code(s, cls, ctb);
        CgenSupport.emitPush(CgenSupport.ACC, s); // 将e1运算后的结果压栈
        ctb.vitualFrame.push("**No**"); /* 栈是要随时模拟的! */

        e2.code(s, cls, ctb); /* 计算后的结果放在$a0之中 */
        // 这么说,我们要另外地构建一个Int对象,要复制的对象的指针放在$a0之中,然后结果的指针也放在$a0之中
        CgenSupport.emitJal("Object.copy", s);
        CgenSupport.emitLoad(CgenSupport.T2, 3, CgenSupport.ACC, s); //  取出实际的整数值,在Int对象偏移量12的位置
        CgenSupport.emitLoad(CgenSupport.T1, 1, CgenSupport.SP, s); // get e1's result
        CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, 4, s); // 退栈
        ctb.vitualFrame.pop(); /* 一定要同时记得退栈 */

        CgenSupport.emitLoad(CgenSupport.T1, 3, CgenSupport.T1, s); // 取出实际的值
        CgenSupport.emitDiv(CgenSupport.T1, CgenSupport.T1, CgenSupport.T2, s);
        CgenSupport.emitStore(CgenSupport.T1, 3, CgenSupport.ACC, s);
        /* 我要说的一句是,结果确实是存放在$a0之中 */
    }


}


/**
 * Defines AST constructor 'neg'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class neg extends Expression {
    public Expression e1;

    /**
     * Creates "neg" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for e1
     */
    public neg(int lineNumber, Expression a1) {
        super(lineNumber);
        e1 = a1;
    }

    public TreeNode copy() {
        return new neg(lineNumber, (Expression) e1.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "neg\n");
        e1.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_neg");
        e1.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        s.println("# 现在开始要编码负数了!");
        e1.code(s, cls, ctb);
        CgenSupport.emitJal("Object.copy", s);
        s.println("# 取出实际的值!");
        CgenSupport.emitLoad(CgenSupport.T1, 3, CgenSupport.ACC, s);
        s.println("# 取反!");
        CgenSupport.emitNeg(CgenSupport.T1, CgenSupport.T1, s);
        s.println("# 将结果放入新的值中!");
        CgenSupport.emitStore(CgenSupport.T1, 3, CgenSupport.ACC, s);
    }


}


/**
 * Defines AST constructor 'lt'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class lt extends Expression {
    public Expression e1;
    public Expression e2;

    /**
     * Creates "lt" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for e1
     * @param a2         initial value for e2
     */
    public lt(int lineNumber, Expression a1, Expression a2) {
        super(lineNumber);
        e1 = a1;
        e2 = a2;
    }

    public TreeNode copy() {
        return new lt(lineNumber, (Expression) e1.copy(), (Expression) e2.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "lt\n");
        e1.dump(out, n + 2);
        e2.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_lt");
        e1.dump_with_types(out, n + 2);
        e2.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        e1.code(s, cls, ctb);
        CgenSupport.emitLoad(CgenSupport.T3, 3, CgenSupport.ACC, s);
        e2.code(s, cls, ctb);
        CgenSupport.emitLoad(CgenSupport.T4, 3, CgenSupport.ACC, s);
        CgenSupport.emitLoadAddress(CgenSupport.ACC, "bool_const1", s);
        int label = CgenSupport.getLabel();
        CgenSupport.emitBlt(CgenSupport.T3, CgenSupport.T4, label, s);
        CgenSupport.emitLoadAddress(CgenSupport.ACC, "bool_const0", s);
        CgenSupport.emitLabelDef(label, s);
    }


}


/**
 * Defines AST constructor 'eq'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class eq extends Expression {
    public Expression e1;
    public Expression e2;

    /**
     * Creates "eq" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for e1
     * @param a2         initial value for e2
     */
    public eq(int lineNumber, Expression a1, Expression a2) {
        super(lineNumber);
        e1 = a1;
        e2 = a2;
    }

    public TreeNode copy() {
        return new eq(lineNumber, (Expression) e1.copy(), (Expression) e2.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "eq\n");
        e1.dump(out, n + 2);
        e2.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_eq");
        e1.dump_with_types(out, n + 2);
        e2.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        e1.code(s, cls, ctb);
        CgenSupport.emitPush(CgenSupport.ACC, s); // 将e1的计算结果压栈
        ctb.vitualFrame.push("~-NO-~");
        e2.code(s, cls, ctb); // 继续来计算e2
        CgenSupport.emitMove(CgenSupport.T2, CgenSupport.ACC, s);
        CgenSupport.emitLoad(CgenSupport.T1, 1, CgenSupport.SP, s);
        CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, 4, s); /* 出栈 */
        ctb.vitualFrame.pop();
        AbstractSymbol sb = e1.get_type();
        if (sb.equals(TreeConstants.Int) || sb.equals(TreeConstants.Str) || sb.equals(TreeConstants.Bool)) {
            System.out.println("eq --> hi");
            /**
             * 这里我稍微要讲一下比较的规则,我们先要将要比较的对象放于$t1和$t2两个寄存器里面,然后调用
             * equality_test这个函数,如果比较两者相同,则输出的东西和$a0中原有的东西相同,否则$a0中的东西是
             * $a1中的东西.对输出的结果放在$a0中
             * 此外 acc代表的就是$a0这个寄存器
             */
            CgenSupport.emitLoadAddress(CgenSupport.ACC, "bool_const1", s); // true
            CgenSupport.emitLoadAddress(CgenSupport.A1, "bool_const0", s); // false
            CgenSupport.emitJal("equality_test", s); /* 如果两者相同,则$a0中的值是true,否则是false */
        } else {
            /* 如果不是基本的类型 */
            int label = CgenSupport.getLabel();
            CgenSupport.emitLoadAddress(CgenSupport.ACC, "bool_const1", s);
            CgenSupport.emitBeq(CgenSupport.T1, CgenSupport.T2, label, s); /* 直接比较地址,这样好吗? */
            CgenSupport.emitLoadAddress(CgenSupport.ACC, "bool_const0", s);
            CgenSupport.emitLabelDef(label, s);
        }
    }


}


/**
 * Defines AST constructor 'leq'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class leq extends Expression {
    public Expression e1;
    public Expression e2;

    /**
     * Creates "leq" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for e1
     * @param a2         initial value for e2
     */
    public leq(int lineNumber, Expression a1, Expression a2) {
        super(lineNumber);
        e1 = a1;
        e2 = a2;
    }

    public TreeNode copy() {
        return new leq(lineNumber, (Expression) e1.copy(), (Expression) e2.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "leq\n");
        e1.dump(out, n + 2);
        e2.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_leq");
        e1.dump_with_types(out, n + 2);
        e2.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        e1.code(s, cls, ctb);
        CgenSupport.emitLoad(CgenSupport.T3, 3, CgenSupport.ACC, s);
        e2.code(s, cls, ctb);
        CgenSupport.emitLoad(CgenSupport.T4, 3, CgenSupport.ACC, s);
        int label = CgenSupport.getLabel();
        CgenSupport.emitLoadAddress(CgenSupport.ACC, "bool_const1", s);
        CgenSupport.emitBleq(CgenSupport.T3, CgenSupport.T4, label, s);
        CgenSupport.emitLoadAddress(CgenSupport.ACC, "bool_const0", s);
        CgenSupport.emitLabelDef(label, s);
    }


}


/**
 * Defines AST constructor 'comp'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class comp extends Expression {
    public Expression e1;

    /**
     * Creates "comp" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for e1
     */
    public comp(int lineNumber, Expression a1) {
        super(lineNumber);
        e1 = a1;
    }

    public TreeNode copy() {
        return new comp(lineNumber, (Expression) e1.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "comp\n");
        e1.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_comp");
        e1.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        s.println("# 从现在开始not表达式的转换,首先计算表达式e1!");
        e1.code(s, cls, ctb);
        s.println("# 总之bool值也是Int类型的,下面取得实际的值");
        CgenSupport.emitLoad(CgenSupport.T1, 3, CgenSupport.ACC, s);
        s.println("# 现在将true加载进a0");
        CgenSupport.emitLoadAddress(CgenSupport.ACC, "bool_const1", s);
        int label = CgenSupport.getLabel();
        s.println("# 如果计算的结果是0的话,那么就将true放入a0中");
        CgenSupport.emitBeqz(CgenSupport.T1, label, s);
        s.println("# 否则的话,就将false放入a0之中");
        CgenSupport.emitLoadAddress(CgenSupport.ACC, "bool_const0", s);
        CgenSupport.emitLabelDef(label, s);
    }

}


/**
 * Defines AST constructor 'int_const'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class int_const extends Expression {
    public AbstractSymbol token;

    /**
     * Creates "int_const" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for token
     */
    public int_const(int lineNumber, AbstractSymbol a1) {
        super(lineNumber);
        token = a1;
    }

    public TreeNode copy() {
        return new int_const(lineNumber, copy_AbstractSymbol(token));
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "int_const\n");
        dump_AbstractSymbol(out, n + 2, token);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_int");
        dump_AbstractSymbol(out, n + 2, token);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method method is provided
     * to you as an example of code generation.
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        /* 下面的代码是已有的吧! */
        CgenSupport.emitLoadInt(CgenSupport.ACC,
                (IntSymbol) AbstractTable.inttable.lookup(token.getString()), s);
    }

}


/**
 * Defines AST constructor 'bool_const'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class bool_const extends Expression {
    public Boolean val;

    /**
     * Creates "bool_const" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for val
     */
    public bool_const(int lineNumber, Boolean a1) {
        super(lineNumber);
        val = a1;
    }

    public TreeNode copy() {
        return new bool_const(lineNumber, copy_Boolean(val));
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "bool_const\n");
        dump_Boolean(out, n + 2, val);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_bool");
        dump_Boolean(out, n + 2, val);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method method is provided
     * to you as an example of code generation.
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        CgenSupport.emitLoadBool(CgenSupport.ACC, new BoolConst(val), s);
    }

}


/**
 * Defines AST constructor 'string_const'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class string_const extends Expression {
    public AbstractSymbol token;

    /**
     * Creates "string_const" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for token
     */
    public string_const(int lineNumber, AbstractSymbol a1) {
        super(lineNumber);
        token = a1;
    }

    public TreeNode copy() {
        return new string_const(lineNumber, copy_AbstractSymbol(token));
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "string_const\n");
        dump_AbstractSymbol(out, n + 2, token);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_string");
        out.print(Utilities.pad(n + 2) + "\"");
        Utilities.printEscapedString(out, token.getString());
        out.println("\"");
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method method is provided
     * to you as an example of code generation.
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        CgenSupport.emitLoadString(CgenSupport.ACC,
                (StringSymbol) AbstractTable.stringtable.lookup(token.getString()), s);
    }

}


/**
 * Defines AST constructor 'new_'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class new_ extends Expression {
    public AbstractSymbol type_name;

    /**
     * Creates "new_" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for type_name
     */
    public new_(int lineNumber, AbstractSymbol a1) {
        super(lineNumber);
        type_name = a1;
    }

    public TreeNode copy() {
        return new new_(lineNumber, copy_AbstractSymbol(type_name));
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "new_\n");
        dump_AbstractSymbol(out, n + 2, type_name);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_new");
        dump_AbstractSymbol(out, n + 2, type_name);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        /* new表达式 */
        s.println("# 下面是new表达式的语句!");
        if (type_name.equals(TreeConstants.SELF_TYPE)) {
            /* 如果要构造出一个自己 */
            CgenSupport.emitLoadAddress(CgenSupport.T1, "class_objTab", s); /* 加载class_objTab */
            CgenSupport.emitLoad(CgenSupport.T2, 0, CgenSupport.SELF, s); /* 获取class-tag */
            /**
             * 这里我必须要说一声,那就是,new SELF_TYPE的语义,这个SELF_TYPE指代的是Object本身的类型,而Object本身的类型
             * 由class tag来标记.
             */
            CgenSupport.emitSll(CgenSupport.T2, CgenSupport.T2, 3, s);
            CgenSupport.emitAddu(CgenSupport.T1, CgenSupport.T1, CgenSupport.T2, s);

            /**
             *  调用Object.copy函数有一个约定,那就是要将原型的指针放在$a0之中,然后
             *  Object.copy函数也会保证说,新的Object的指针也会放在$a0之中,这里需要说一声,
             *  那就是这个函数的运行会改变t0, t1, a0, a1等寄存器的值,所以如果这些寄存器里面有
             *  重要的数据的话,那么一定要记得压栈.
             *  */
            CgenSupport.emitPush(CgenSupport.T1, s); /* 压栈, t1中存放的是类的原型的首址 */
            ctb.vitualFrame.push("**No**");

            CgenSupport.emitLoad(CgenSupport.ACC, 0, CgenSupport.T1, s);
            CgenSupport.emitJal("Object.copy", s);
            /**
             * lw rt address    我稍微来讲解一下指令的作用,从address所指的地方取出一个word的数据放到rt所代表的寄存器中.
             * address一般这么来表示, immd(rt),应该很简单吧,其实就是rt所代表的寄存器里的地址加上immd所代表的立即数组成的.
             */
            CgenSupport.emitLoad(CgenSupport.T1, 1, CgenSupport.SP, s); /* 恢复原来t1寄存器的值 */
            /**
             * 接下来要运行初始化的函数了,很简单,我们从原来的objTab中也可以看得到,那就是初始化函数的标号恰好在
             * 类原型的标号的后面.
             */
            CgenSupport.emitLoad(CgenSupport.T1, 1, CgenSupport.T1, s); /* 由于类的原型的标号后一个地址就是初始化函数的标号,移动一步 */
            CgenSupport.emitAddiu(CgenSupport.SP, CgenSupport.SP, 4, s); /* 记得出栈 */
            ctb.vitualFrame.pop();

            /**
             * 一般来说,在调用函数之前,一定要记得将self的指针放在a0之中,这是一个约定.这里的a0中存放的是新构建的对象的指针,所以初始化的
             * 是新构建的对象
             */
            CgenSupport.emitJalr(CgenSupport.T1, s); /* 跳转到初始化函数去执行 */
        } else {
            /**
             *  否则的话,要构建的就是任意的类型的对象了,这样一来,其实代码会更加简单,因为类型已经知道了
             *  那么它们的原型标号和初始化函数的标号我们也已经知晓了.
             */
            String protTyp = type_name.str + CgenSupport.PROTOBJ_SUFFIX;
            String initSym = type_name.str + CgenSupport.CLASSINIT_SUFFIX;
            CgenSupport.emitLoadAddress(CgenSupport.ACC, protTyp, s);
            CgenSupport.emitJal("Object.copy", s);
            s.println("# 现在,新的" + type_name.str + "对象的指针在a0之中,接下来要初始化这个新的对象!");
            CgenSupport.emitJal(initSym, s);
        }
    }


}


/**
 * Defines AST constructor 'isvoid'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class isvoid extends Expression {
    public Expression e1;

    /**
     * Creates "isvoid" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a0         initial value for e1
     */
    public isvoid(int lineNumber, Expression a1) {
        super(lineNumber);
        e1 = a1;
    }

    public TreeNode copy() {
        return new isvoid(lineNumber, (Expression) e1.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "isvoid\n");
        e1.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_isvoid");
        e1.dump_with_types(out, n + 2);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
    }


}


/**
 * Defines AST constructor 'no_expr'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class no_expr extends Expression {
    /**
     * Creates "no_expr" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     */
    public no_expr(int lineNumber) {
        super(lineNumber);
    }

    public TreeNode copy() {
        return new no_expr(lineNumber);
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "no_expr\n");
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_no_expr");
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
    }


}


/**
 * Defines AST constructor 'object'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class object extends Expression {
    public AbstractSymbol name;

    /**
     * Creates "object" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     */
    public object(int lineNumber, AbstractSymbol a1) {
        super(lineNumber);
        name = a1;
    }

    public TreeNode copy() {
        return new object(lineNumber, copy_AbstractSymbol(name));
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "object\n");
        dump_AbstractSymbol(out, n + 2, name);
    }

    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_object");
        dump_AbstractSymbol(out, n + 2, name);
        dump_type(out, n);
    }

    /**
     * Generates code for this expression.  This method is to be completed
     * in programming assignment 5.  (You may add or remove parameters as
     * you wish.)
     *
     * @param s the output stream
     */
    public void code(PrintStream s, class_ cls, CgenClassTable ctb) {
        // 如果要写解释器的话,我们要到env中寻找对应的变量
        s.println("# 现在正在寻找变量" + name.str + "!");
        if (get_type().str.equals("SELF_TYPE")) {
            // 也就是说,其实要找的只是自己罢了,我们将self放入结果$a0中
            s.println("# 要求的是SELF_TYPE,所以要找的变量就是自己,所以将self的指针放入a0之中!");
            CgenSupport.emitMove(CgenSupport.ACC, CgenSupport.SELF, s);
            return;
        }
        int offset = 0;
        /**
         * 这里,我需要说明一下,Object引用的变量无非在三个地方,一个是在let表达式中,一个是在函数的参数之中,
         * 要不就是某个Object的attr,所以对于三种引用,我们都要能够应付自如.
         */
        // 如果表达式在let表达式中,注意,let表达式是可以嵌套的
        for (int i = ctb.vitualFrame.size() - 1; i >= 0; --i) {
            String var = ctb.vitualFrame.get(i);
            if (var.equals(name.str)) {
                /* 找到了 */
                offset = ctb.vitualFrame.size() - i;
                CgenSupport.emitLoad(CgenSupport.ACC, offset, CgenSupport.SP, s);
                return;
            }

        }
        Object res = ctb.argMp.get(name.str); /* 我这里要求的是某个变量的偏移地址 */
        if (res != null) {
            /* 在函数的参数之中 */
            offset = (int) res;
            /* 我们现在要求fp距离参数的偏移量(单位是word) */
            offset = ctb.argMp.size() - 1 + 3 - offset;
            CgenSupport.emitLoad(CgenSupport.ACC, offset, CgenSupport.FP, s);
        } else {
            /* 这个变量是Object的attr */
            String clsName = cls.getName().str;
            offset = 3 + ctb.attrTab.getClsAttrOffset(clsName, name.str);
            //offset = 3 + ctb.attrTab.get(clsName).get(clsName + "." + name.str);
            System.out.println("object --> " + clsName + "的属性" + name.str + "的偏移地址是" + offset * 4);
            s.println("# 我们要寻找的这个变量一定是这个变量的attr!");
            CgenSupport.emitLoad(CgenSupport.ACC, offset, CgenSupport.SELF, s);
        }
    }
}


