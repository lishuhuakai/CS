// -*- mode: java -*- 
//
// file: cool-tree.m4
//
// This file defines the AST
//
//////////////////////////////////////////////////////////

import java.util.*;
import java.io.PrintStream;

/* 不得不说,生成的这么多的node还是挺漂亮的! */

/**
 * Defines simple phylum Program
 */
abstract class Program extends TreeNode {
    protected Program(int lineNumber) {
        super(lineNumber);
    }

    public abstract void dump_with_types(PrintStream out, int n);

    public abstract void semant(); /* 这个函数时要求自己去实现的! */

}


/**
 * Defines simple phylum Class_
 */
abstract class Class_ extends TreeNode {
    protected Class_(int lineNumber) {
        super(lineNumber);
    }

    public abstract void dump_with_types(PrintStream out, int n);

}


/**
 * Defines list phylum Classes
 * <p>
 * See <a href="ListNode.html">ListNode</a> for full documentation.
 */
class Classes extends ListNode {
    /* 果然没有猜错,这个玩意继承自ListNode,也就是说包含很多的classes */
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

    abstract public void typeCheck(ClassTable ct, SymbolTable tenv, class_c cls); /* 这个要由子类来实现 */
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

    abstract public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls);
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
 * Defines AST constructor 'programc'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class programc extends Program {
    protected Classes classes;

    /**
     * Creates "programc" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for classes
     */
    public programc(int lineNumber, Classes a1) {
        super(lineNumber);
        classes = a1;
    }

    public TreeNode copy() {

        return new programc(lineNumber, (Classes) classes.copy());
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "programc\n");
        classes.dump(out, n + 2);
    }


    public void dump_with_types(PrintStream out, int n) {
        dump_line(out, n);
        out.println(Utilities.pad(n) + "_program");
        for (Enumeration e = classes.getElements(); e.hasMoreElements(); ) {
            // sm: changed 'n + 1' to 'n + 2' to match changes elsewhere
            ((Class_) e.nextElement()).dump_with_types(out, n + 2);
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
    public void semant() { /* 好吧,这里就是所谓的起点啊! */
    /* ClassTable constructor may do some semantic analysis */
        ClassTable classTable = new ClassTable();

    /* 我要说的是classes由很多的class组成!所以所要遍历树! */
        Enumeration en = classes.getElements();
        /* first of all, construct a InheritGraph */
        while (en.hasMoreElements()) {
        /* 这可是第一遍, pass */
            class_c cls = (class_c) en.nextElement();
            classTable.addClasses(cls);
            classTable.hg.addEdge(cls.getName().getString(), cls.getParent().getString());
        }

        /* 需要定义Main这个类 */
        if (classTable.getClass("Main") == null) {
            classTable.semantError().println("Class Main is not defined.");
        }
        /* 第二遍扫描*/
        en = classes.getElements();
        while (en.hasMoreElements()) {
            class_c cls = (class_c) en.nextElement();
            classTable.hg.addEdge(cls.getName().getString(), cls.getParent().getString());
        }
        /* 第三遍扫描 */
        en = classes.getElements();
        while (en.hasMoreElements()) {
            class_c cls = (class_c) en.nextElement();
            classTable.hg.addEdge(cls.getName().getString(), cls.getParent().getString());
        }
        //System.out.println("--------如花美眷,似水流年!--------");
        //classTable.hg.printGraph();
        //System.out.println("----------------------------------");
    /* some semantic analysis code may go here */
        en = classes.getElements();
        SymbolTable map = new SymbolTable();
        while (en.hasMoreElements()) {
            class_c cls = (class_c) en.nextElement();
            cls.typeCheck(classTable, map);
        }
        if (classTable.errors()) {
            System.err.println("Compilation halted due to static semantic errors.");
            System.exit(1);
        }
    }

}


/**
 * Defines AST constructor 'class_c'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class class_c extends Class_ {
    protected AbstractSymbol name;
    protected AbstractSymbol parent;
    protected Features features;
    protected AbstractSymbol filename;

    /**
     * Creates "class_c" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     * @param a1         initial value for parent
     * @param a2         initial value for features
     * @param a3         initial value for filename
     */

    public class_c(int lineNumber, AbstractSymbol a1, AbstractSymbol a2, Features a3, AbstractSymbol a4) {
        super(lineNumber);
        name = a1;
        parent = a2;
        features = a3;
        filename = a4;
    }

    public TreeNode copy() {
        return new class_c(lineNumber, copy_AbstractSymbol(name), copy_AbstractSymbol(parent), (Features) features.copy(), copy_AbstractSymbol(filename));
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "class_c\n");
        dump_AbstractSymbol(out, n + 2, name);
        dump_AbstractSymbol(out, n + 2, parent);
        features.dump(out, n + 2);
        dump_AbstractSymbol(out, n + 2, filename);
    }

    public AbstractSymbol getFilename() {
        return filename;
    }

    public AbstractSymbol getName() {

        return name;
    }

    public AbstractSymbol getParent() {
        return parent;
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


    private Stack<String> getParentList(ClassTable ct, class_c parent) {
        Stack<String> stk = new Stack<>();
        if (parent == null)
            return stk;
        else
            stk.push(parent.name.str);
        //System.out.println(parent.name.str);
        parent = ct.getClass(parent.parent.str);
        while (parent != null) {
            stk.push(parent.name.str);
            parent = ct.getClass(parent.parent.str);
        }
        return stk;
    }

    private void addParentAttr(ClassTable ct, SymbolTable tenv, class_c cls) {
        if (cls == null) return;
        Stack<String> stk = getParentList(ct, ct.getClass(cls.parent.str));
        while (!stk.empty()) {
            class_c parent = ct.getClass(stk.pop());
            Enumeration en = parent.features.getElements();
            while (en.hasMoreElements()) {
                Object obj = en.nextElement();
                if (obj instanceof attr) {
                    attr atr = (attr) obj;
                    tenv.addId(atr.name, atr.type_decl.str);
                }
            }
        }
    }

    private boolean isParentIllegal(String parent) {
        if (parent.equals("Bool") || parent.equals("SELF_TYPE") || parent.equals("String")) {
            return true;
        }
        return false;
    }

    public void typeCheck(ClassTable ct, SymbolTable tenv) {
        /* 一些类是不能够被继承的 */
        if (isParentIllegal(parent.str)) {
            ct.semantError().println(this.filename + ":" + lineNumber + ": Class " +
                    name.str + " cannot inherit class " + parent.str + " .");
        }

        if (name.str.equals("SELF_TYPE")) {
            ct.semantError().println(this.filename + ":" + lineNumber +
                    ": Redefinition of basic class " + name.str + " .");
        }
        /* 被继承的类应该是存在的 */
        if (ct.getClass(parent.str) == null) {
            ct.semantError().println(this.filename + ":" + lineNumber + ": Class " +
                    name.str + " inherits from an undefined class " + parent.str + " .");
        }

        tenv.enterScope();
        addParentAttr(ct, tenv, this);
        Enumeration en = features.getElements(); /* 得到一个迭代器 */
        while (en.hasMoreElements()) {
            Feature feature = (Feature) en.nextElement();
            feature.typeCheck(ct, tenv, this); /* 好吧,将自己传递了过去! */
        }
        tenv.exitScope();
    }

}


/**
 * Defines AST constructor 'method'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class method extends Feature {
    protected AbstractSymbol name;
    protected Formals formals;
    protected AbstractSymbol return_type;
    protected Expression expr;

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
        return new method(lineNumber, copy_AbstractSymbol(name), (Formals) formals.copy(),
                copy_AbstractSymbol(return_type), (Expression) expr.copy());
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

    private void verifyOverride(ClassTable ct, SymbolTable tenv, class_c cls, method childMtd) {
        /* 这个函数用于检查函数的覆写是否符合规则 */
        method mtd = common.findParentMethod(ct, cls, childMtd.name.str);
        if (mtd != null) {
            // 比较形参,如果参数类型不同,则报错!
            Enumeration cEn = childMtd.formals.getElements();
            Enumeration pEn = mtd.formals.getElements();
            while (cEn.hasMoreElements() && pEn.hasMoreElements()) {
                formalc pf = (formalc) pEn.nextElement();
                formalc cf = (formalc) cEn.nextElement();
                if (!pf.type_decl.str.equals(cf.type_decl.str)) {
                    ct.semantError().println(cls.filename + ":" + lineNumber + ": In redefined method " +
                    childMtd.name.str + " , parameter type " + cf.type_decl.str + " is different from original type " +
                    pf.type_decl.str + " .");
                }
            }
            if (cEn.hasMoreElements() || pEn.hasMoreElements()) {
                // 先写在这里,留一个坑
                ct.semantError().println(cls.filename + ":" + lineNumber +
                        ": Incompatible number of formal parameters in redefined method " + childMtd.name.str +
                " .");
            }
        }
    }

    public void typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        /* 首先要得到参数的列表 */
        Enumeration en = formals.getElements();
        //System.out.println(name);
        tenv.enterScope();
        verifyOverride(ct, tenv, cls, this);
        tenv.addId(TreeConstants.self, cls.name.str);
        HashSet<String> args = new HashSet<>();
        while (en.hasMoreElements()) {
            /* 现在开始将这些参数加入tenv中 */
            formalc fm = (formalc) en.nextElement();
            if (fm.name.str.equals("self")) {
                ct.semantError().println(cls.filename + ":" + lineNumber +
                        ": 'self' cannot be the name of a formal parameter");
            }

            if (fm.type_decl.str.equals("SELF_TYPE")) {
                ct.semantError().println(cls.filename + ":" + lineNumber + ": " +
                        "Formal parameter " + fm.name.str + "cannot have type SELF_TYPE.");
            }
            tenv.addId(fm.name, fm.type_decl.str);
            if (args.contains(fm.name.str)) {
                ct.semantError().println(cls.filename + ":" + lineNumber + ": Formal parameter " +
                        fm.name.str + " is multiply defined.");
            } else {
                args.add(fm.name.str);
            }
        }
        String retnType = return_type.str;
        //System.out.println("retnType --> " + retnType);
        String exprType = expr.typeCheck(ct, tenv, cls); // 得到expr的类型
        //System.out.println("exprType --> " + exprType);
        if (!ct.typeExist(retnType) && !retnType.equals("SELF_TYPE")) {
            // 要考虑的问题有点多,如果返回值的类型不存在
            //ct.semantError().println("line " + lineNumber + " : " +
            //        "type " + retnType + " not exist!");
        }
        else {
            if (retnType.equals("SELF_TYPE")) {
                //System.out.println(expr.get_type().str);
                if (!expr.get_type().str.equals("SELF_TYPE")) {
                    ct.semantError().println(cls.filename + ":" + lineNumber + ": Inferred return type" +
                    exprType + " of method " + name.str +
                            " does not conform to declared return type SELF_TYPE.");
                }
                retnType = cls.name.str;
            }
            if (exprType.equals("SELF_TYPE")) {
                exprType = cls.name.str;
            }
            // 现在要求exprType是retnType的子类
            if (!ct.hg.isDescendentOf(exprType, retnType))
                ct.semantError().println(cls.filename + ":" + lineNumber + ": Inferred return type " +
                        exprType + " of method " + name.str + " oes not conform to declared return type " +
                        retnType + " .");
        }
        tenv.exitScope();
    }
}


/**
 * Defines AST constructor 'attr'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class attr extends Feature {
    protected AbstractSymbol name;
    protected AbstractSymbol type_decl;
    protected Expression init;

    /**
     * Creates "attr" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     * @param a1         initial value for type_decl
     * @param a2         initial value for init
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

    private boolean variableExist(String varName, ClassTable ct, class_c cls) {
        class_c fatherClass = ct.getClass(cls.parent.str); /* 先找到父类对应的类 */
        if (fatherClass == null) return false;
        Enumeration en = fatherClass.features.getElements();
        while (en.hasMoreElements()) {
            Feature ft = (Feature) en.nextElement();
            if (ft instanceof attr) {
                attr at = (attr) ft;
                if (at.name.str.equals(varName))
                    return true;
            }
        }
        // 继续递归调用
        return variableExist(varName, ct, fatherClass);
    }

    public void typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        /* 这里进入的属性表达式! */
        // 这里还有一项非常重要的任务,那就是看父类中是否也存在该类型的定义.
        String key = name.getString();
        //System.out.println(key);
        if (variableExist(key, ct, cls)) {
            ct.semantError().println(cls.filename.str + ":" + lineNumber + ": " +
                    "Attribute " + key + " is an attribute of an inherited class.");
        }

        if (key.equals("self")) {
            ct.semantError().println(cls.filename.str + ":" + lineNumber +
                    ": " + "'self' cannot be the name of an attribute");
        }
        String type = type_decl.getString();
        //System.out.println(type);
        tenv.addId(name, type);
        if (!(init instanceof no_expr)) {
            tenv.addId(AbstractTable.idtable.addString("self"), cls.name.str);
            String exprType = init.typeCheck(ct, tenv, cls);
            //System.out.println(exprType);
            if (!ct.hg.isDescendentOf(exprType, type))
                ct.semantError().println(key + " has wrong type!");
        }
    }
}


/**
 * Defines AST constructor 'formalc'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class formalc extends Formal {
    protected AbstractSymbol name;
    protected AbstractSymbol type_decl;

    /**
     * Creates "formalc" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     * @param a1         initial value for type_decl
     */
    public formalc(int lineNumber, AbstractSymbol a1, AbstractSymbol a2) {
        super(lineNumber);
        name = a1;
        type_decl = a2;
    }

    public TreeNode copy() {
        return new formalc(lineNumber, copy_AbstractSymbol(name), copy_AbstractSymbol(type_decl));
    }

    public void dump(PrintStream out, int n) {
        out.print(Utilities.pad(n) + "formalc\n");
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
    protected AbstractSymbol name;
    protected AbstractSymbol type_decl;
    protected Expression expr;

    /**
     * Creates "branch" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     * @param a1         initial value for type_decl
     * @param a2         initial value for expr
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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        // 好的,终于到了branch分支了!
        //System.out.println("branch");
        tenv.enterScope();
        tenv.addId(name, type_decl.str);
        String exprType = expr.typeCheck(ct, tenv, cls);
        tenv.exitScope();
        return exprType;
    }

}


/**
 * Defines AST constructor 'assign'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class assign extends Expression {
    protected AbstractSymbol name;
    protected Expression expr;

    /**
     * Creates "assign" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for name
     * @param a1         initial value for expr
     */
    public assign(int lineNumber, AbstractSymbol a1, Expression a2) {
        super(lineNumber);
        name = a1; /* name是什么东西? */
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
        dump_type(out, n); /* 这个玩意是干什么的呢? */
    }

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        /* 现在来解决赋值的问题 */
        if (name.str.equals("self")) {
            // 不能给self赋值
            ct.semantError().println(cls.filename + ":" + lineNumber + ": Cannot assign to 'self'.");
        }
        String varType = (String) tenv.lookup(name); // 查找变量的类型
        String valType = expr.typeCheck(ct, tenv, cls); // 赋值表达式的类型
        //System.out.println("valType ->  " + valType);
        // x : B 且 B是A的子类
        // x <- new A
        // A是valType 而B是varType
        // 事实上,这个式子是错误的.
        // 我们要求valType是varType的子类
        if (!ct.hg.isDescendentOf(valType, varType)) {
            ct.semantError().println(cls.filename.str + ":" + lineNumber + ": Type " + varType +
                    " of assigned expression does not conform to declared type " + valType
                    + " of identifier " + name + " .");
            return "Object";
        }
        //在这里要标注类型!
        //System.out.println(valType);
        set_type(AbstractTable.idtable.addString(valType));
        return valType;
    }

    public String toString() {
        return name.toString() + " <- " + expr.toString() + " ";
    }
}


/**
 * Defines AST constructor 'static_dispatch'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class static_dispatch extends Expression {
    protected Expression expr;
    protected AbstractSymbol type_name;
    protected AbstractSymbol name;
    protected Expressions actual;

    /**
     * Creates "static_dispatch" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for expr
     * @param a1         initial value for type_name
     * @param a2         initial value for name
     * @param a3         initial value for actual
     */
    public static_dispatch(int lineNumber, Expression a1, AbstractSymbol a2, AbstractSymbol a3, Expressions a4) {
        super(lineNumber);
        expr = a1;
        type_name = a2;
        name = a3;
        actual = a4;
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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        //System.out.println("HI");
        String selfType = expr.typeCheck(ct, tenv, cls);

        String classType = type_name.str;
        /**
         * 以 b@A.mtd(); 为例,假设b的类型是B,我们要保证B是A的子类
         */
        //System.out.println(selfType);
        //System.out.println(classType);
        //System.out.println(ct.hg.lub(selfType, classType));
        //System.out.println(ct.hg.isDescendentOf(selfType, classType));
        if (!ct.hg.isDescendentOf(selfType, classType)) {
            ct.semantError().println(cls.filename + ":" + lineNumber + ": " +
                    "Expression type " + selfType +
            " does not conform to declared static dispatch type " + classType + " .");
        }
        /**
         * 然后找到这个类!
         */
        method mtd = common.getMethod(ct, ct.getClass(classType), name.str);

        if (mtd == null) {
            System.out.println("hi");
            ct.semantError().println(cls.filename + ":" + lineNumber + ": Expression " + expr.toString()
                    + " does not conform to declared static dispatch type " + classType);
            set_type(AbstractTable.idtable.addString("Object"));
            return "Object";
        }
        /**
         * 然后我们就要开始验证了!
         * 首先参数的个数应该是一致的.
         */
        String returnType = mtd.return_type.str;
        Enumeration fen = mtd.formals.getElements();
        Enumeration aen = actual.getElements();
        // 实际参数和形式参数的比较
        while (fen.hasMoreElements() && aen.hasMoreElements()) {
            Expression expr = (Expression) aen.nextElement();
            String exprType = expr.typeCheck(ct, tenv, cls); /* 得到实际的参数类型 */
            //System.out.println("exprType -> " + exprType);
            formalc formal = (formalc) fen.nextElement();
            String formalType = formal.type_decl.str; /* 得到形式参数的类型! */
            //System.out.println("formalType -> " + formalType);
            // 要求实参是形参的子类才行
            if (!ct.hg.isDescendentOf(exprType, formalType)) {
                ct.semantError().println(cls.filename + ":" + lineNumber + ": " + "In call of method "
                        + mtd.name.str + " type " + exprType + " of parameter " + formal.name +
                        " does not conform to declared type " + formal.type_decl + " .");
            }
        }

        if (fen.hasMoreElements()) {
            // 运行到这一步的话,说明实参有点多
            ct.semantError().println(cls.filename + ":" + lineNumber + ": " + "wrong!");
        }

        if (aen.hasMoreElements()) {
            ct.semantError().println(cls.filename + ":" + lineNumber + ": " + "wrong!");
        }
        // 好了,这来看一看返回值的类型吧!
        String res;
        if (returnType.equals("SELF_TYPE"))
            res = selfType;
        else
            res = returnType;
        set_type(AbstractTable.idtable.addString(res));
        return res;
    }
}


/**
 * Defines AST constructor 'dispatch'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class dispatch extends Expression {
    protected Expression expr;
    protected AbstractSymbol name;
    protected Expressions actual;

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
        return new dispatch(lineNumber, (Expression) expr.copy(),
                copy_AbstractSymbol(name), (Expressions) actual.copy());
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

    private method findMethod(ClassTable ct, class_c cls, String methodName) {
        if (cls == null) return null;
        Enumeration en = cls.features.getElements();
        while (en.hasMoreElements()) {
            Feature ft = (Feature) en.nextElement();
            if (ft instanceof method) {
                method res = (method) ft;
                if (res.name.str.equals(methodName)) {
                    return res;
                }
            }
        }
        // 否则的话,就要到父类中去找
        return findMethod(ct, ct.getClass(cls.parent.str), methodName);
    }

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        // expr输出的是cow
        String tp = expr.typeCheck(ct, tenv, cls);
        //System.out.println(tp);

        method mtd = findMethod(ct, ct.getClass(tp), name.str);
        //System.out.println(name.str);
        if (mtd == null) {
            ct.semantError().println(cls.filename + ":" + lineNumber + ": Dispatch to undefined method "
                    + name.str);
            set_type(AbstractTable.idtable.addString("Object"));
            return "Object";
        }
        // 然后我们就要开始验证了!
        // 首先参数的个数应该是一致的.
        String returnType = mtd.return_type.str;
        Enumeration fen = mtd.formals.getElements();
        Enumeration aen = actual.getElements();
        // 实际参数和形式参数的比较
        while (fen.hasMoreElements() && aen.hasMoreElements()) {
            Expression expr = (Expression) aen.nextElement();
            String exprType = expr.typeCheck(ct, tenv, cls); /* 得到实际的参数类型 */
            //System.out.println("exprType -> " + exprType);
            formalc formal = (formalc) fen.nextElement();
            String formalType = formal.type_decl.str; /* 得到形式参数的类型! */
            //System.out.println("formalType -> " + formalType);
            // 要求实参是形参的子类才行
            if (!ct.hg.isDescendentOf(exprType, formalType)) {
                ct.semantError().println(cls.filename + ":" + lineNumber + ": " + "In call of method "
                        + mtd.name.str + " type " + exprType + " of parameter " + formal.name +
                        " does not conform to declared type " + formal.type_decl + " .");
            }
        }

        if (fen.hasMoreElements()) {
            // 运行到这一步的话,说明实参有点多
            ct.semantError().println(cls.filename + ":" + lineNumber + ": " + "wrong!");
        }

        if (aen.hasMoreElements()) {
            ct.semantError().println(cls.filename + ":" + lineNumber + ": " + "wrong!");
        }
        // 好了,这来看一看返回值的类型吧!
        // 好吧,这些个玩意其实还是挺复杂的.
        String res;
        //System.out.println("method-name : " + mtd.name.str + " return-type : " + returnType);
        if (returnType.equals("SELF_TYPE")) {
            if (tp.equals(cls.name.str))
                set_type(TreeConstants.SELF_TYPE);
            else // 调用者所属的类和函数所处的类不一样,那么这里的SELF_TYPE指的是调用者所属的类
                set_type(AbstractTable.idtable.addString(tp));
            res = tp;
        } else {
            set_type(AbstractTable.idtable.addString(returnType));
            res = returnType;
        }
        return res;
    }
}


/**
 * Defines AST constructor 'cond'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class cond extends Expression {
    protected Expression pred;
    protected Expression then_exp;
    protected Expression else_exp;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        /* cond 就是所谓的if语句!*/
        String condType = pred.typeCheck(ct, tenv, cls);
        if (!condType.equals("Bool")) {
            ct.semantError().println("line " + lineNumber + " : " + "if pred has wrong type!");
        }
        String thenType = then_exp.typeCheck(ct, tenv, cls);
        String elseType = else_exp.typeCheck(ct, tenv, cls);
        String res = ct.hg.lub(thenType, elseType);
        set_type(AbstractTable.idtable.addString(res));
        return res;
    }
}


/**
 * Defines AST constructor 'loop'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class loop extends Expression {
    protected Expression pred;
    protected Expression body;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        // 好吧,终于到了while表达式了!
        String predType = pred.typeCheck(ct, tenv, cls);
        //System.out.println(predType);
        if (!predType.equals("Bool")) {
            ct.semantError().println(cls.filename + ":" + lineNumber +
                    ": Loop condition does not have type Bool.");
        }
        String bodyType = body.typeCheck(ct, tenv, cls);
        set_type(AbstractTable.idtable.addString("Object"));
        return "Object";
    }

}


/**
 * Defines AST constructor 'typcase'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class typcase extends Expression {
    protected Expression expr;
    protected Cases cases;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        // 好吧,这里才是正宗的case语句
        //System.out.println("HI");
        String tp1 = null;
        String tp2 = null; // 两个类
        int counter = 0;
        String exprType = expr.typeCheck(ct, tenv, cls);
        HashSet<String> record = new HashSet<>();
        //System.out.println("----------------------");
        //ct.hg.rprintGraph();
        //System.out.println("----------------------");
        Enumeration en = cases.getElements();
        while (en.hasMoreElements()) {
            branch brc = (branch) en.nextElement();
            if (counter == 0) {
                tp1 = brc.typeCheck(ct, tenv, cls);
                //System.out.println(tp1);
                record.add(brc.type_decl.str);
                ++counter;
            } else {

                if (record.contains(brc.type_decl.str)) {
                    ct.semantError().println(cls.filename + ":" + lineNumber + ":" +
                            "Duplicate branch " + brc.type_decl.str + " Int in case statement.");
                }
                tp2 = brc.typeCheck(ct, tenv, cls);
                //System.out.println(tp2);
                ++counter;
                //System.out.println("before lub, tp1 = " + tp1);
                //System.out.println("before lub, tp2 = " + tp2);
                tp1 = ct.hg.lub(tp1, tp2);
                //ct.hg.printGraph();
                //System.out.println("lub --> " + ct.hg.lub("Razz", "Bar"));
                //System.out.println("lub --> " + ct.hg.lub("Bar", "Razz"));
            }
        }
        //System.out.println(tp1);
        set_type(AbstractTable.idtable.addString(tp1));
        return tp1;
    }

}


/**
 * Defines AST constructor 'block'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class block extends Expression {
    protected Expressions body;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        /* Yes, We enter block expression! */
        String type = "Object";
        Enumeration en = body.getElements();
        Expression expr = null;
        while (en.hasMoreElements()) {
            expr = (Expression) en.nextElement();
            type = expr.typeCheck(ct, tenv, cls);
        }
        //System.out.println("hi");
        //System.out.println(type);
        set_type(expr.get_type());
        return type;
    }
}


/**
 * Defines AST constructor 'let'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class let extends Expression {
    protected AbstractSymbol identifier;
    protected AbstractSymbol type_decl;
    protected Expression init;
    protected Expression body;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        //System.out.println(init);
        tenv.enterScope();
        if (identifier.str.equals("self")) {
            ct.semantError().println(cls.filename + ":" + lineNumber + ": " +
                    "\'self\' cannot be bound in a \'let\' expression.");
        }
        //System.out.println(type_decl.str);
        if (!(init instanceof no_expr)) {
            // 如果没有初始化的动作
            String initType = init.typeCheck(ct, tenv, cls);
            String declareType = type_decl.str;
            //System.out.println(initType);
            //System.out.println(declareType);
            if (type_decl.str.equals("SELF_TYPE")) {
                declareType = cls.name.str;
                // System.out.println(declareType);
            }
            if (initType.equals("SELF_TYPE")) {
                initType = cls.name.str;
                //System.out.println(initType);
            }
            if (!ct.hg.isDescendentOf(initType, declareType)) {
                ct.semantError().println(cls.filename + ":" + lineNumber + ": Inferred type " +
                        initType + " of initialization of " + identifier.str + " does not conform to " +
                        "identifier's declared type " + declareType);
            }
        }
        // 说实话,这真是挺蛋疼的一件事情!
        //if (type_decl.str.equals("SELF_TYPE"))
        //  tenv.addId(identifier, cls.name.str);
        //else
        tenv.addId(identifier, type_decl.str);

        String exprType = body.typeCheck(ct, tenv, cls);
        tenv.exitScope();
        set_type(body.get_type());
        return exprType;
    }
}


/**
 * Defines AST constructor 'plus'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class plus extends Expression {
    protected Expression e1;
    protected Expression e2;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        /* 现在来检测加法了! */
        String tp1 = e1.typeCheck(ct, tenv, cls);
        String tp2 = e2.typeCheck(ct, tenv, cls);
        boolean flag = false;
        if (!tp1.equals("Int") || !tp2.equals("Int")) {
            flag = true;
            ct.semantError().println(cls.filename + ":" + lineNumber + ": non-Int arguments: " +
                    tp1 + " + " + tp2);
        }

        if (flag) {
            return "Object";
        } else {
            set_type(TreeConstants.Int);
            return "Int";
        }

    }

    public String toString() {
        return e1.toString() + " + " + e2.toString() + " ";
    }

}


/**
 * Defines AST constructor 'sub'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class sub extends Expression {
    protected Expression e1;
    protected Expression e2;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
       /* 现在来检测加法了! */
        String tp1 = e1.typeCheck(ct, tenv, cls);
        String tp2 = e2.typeCheck(ct, tenv, cls);
        boolean flag = false;
        if (!tp1.equals("Int") || !tp2.equals("Int")) {
            flag = true;
            ct.semantError().println(cls.filename + ":" + lineNumber + ": non-Int arguments: " +
                    tp1 + " - " + tp2);
        }

        if (flag) {
            return "Object";
        } else {
            set_type(TreeConstants.Int);
            return "Int";
        }
    }

}


/**
 * Defines AST constructor 'mul'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class mul extends Expression {
    protected Expression e1;
    protected Expression e2;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        String tp1 = e1.typeCheck(ct, tenv, cls);
        String tp2 = e2.typeCheck(ct, tenv, cls);
        boolean flag = false;
        if (!tp1.equals("Int") || !tp2.equals("Int")) {
            flag = true;
            ct.semantError().println(cls.filename + ":" + lineNumber + ": non-Int arguments: " +
                    tp1 + " * " + tp2);
        }

        if (flag) {
            return "Object";
        } else {
            set_type(TreeConstants.Int);
            return "Int";
        }
    }

}


/**
 * Defines AST constructor 'divide'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class divide extends Expression {
    protected Expression e1;
    protected Expression e2;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        String tp1 = e1.typeCheck(ct, tenv, cls);
        String tp2 = e2.typeCheck(ct, tenv, cls);
        boolean flag = false;
        if (!tp1.equals("Int") || !tp2.equals("Int")) {
            flag = true;
            ct.semantError().println(cls.filename + ":" + lineNumber + ": non-Int arguments: " +
                    tp1 + " + " + tp2);
        }

        if (flag) {
            return "Object";
        } else {
            set_type(TreeConstants.Int);
            return "Int";
        }
    }
}


/**
 * Defines AST constructor 'neg'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class neg extends Expression {
    protected Expression e1;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        String exprType = e1.typeCheck(ct, tenv, cls);
        if (!exprType.equals("Int")) {
            ct.semantError().println("Wrong");
            return "Object";
        }
        set_type(TreeConstants.Int);
        return "Int";
    }

}


/**
 * Defines AST constructor 'lt'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class lt extends Expression {
    protected Expression e1;
    protected Expression e2;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        // less than
        String e1Type = e1.typeCheck(ct, tenv, cls);
        String e2Type = e2.typeCheck(ct, tenv, cls);
        if (!(e1Type.equals("Int") && e2Type.equals("Int"))) {
            ct.semantError().println("Wrong!");
        }
        set_type(TreeConstants.Bool);
        return "Bool";
    }
}


/**
 * Defines AST constructor 'eq'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class eq extends Expression {
    protected Expression e1;
    protected Expression e2;

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

    public boolean isBasicType(String tp) {
        if (tp.equals("Int") || tp.equals("String") || tp.equals("Bool")) {
            return true;
        } else {
            return false;
        }
    }

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        // 好吧,终究还是到了这里
        String e1Type = e1.typeCheck(ct, tenv, cls);
        String e2Type = e2.typeCheck(ct, tenv, cls);
        if (isBasicType(e1Type) || isBasicType(e2Type)) {
            if (!e1Type.equals(e2Type)) {
                ct.semantError().println(cls.filename + ":" + lineNumber + ": " +
                        "Illegal comparison with a basic type.");
                set_type(AbstractTable.idtable.addString("Object"));
                return "Object";
            }
        }
        set_type(AbstractTable.idtable.addString("Bool"));
        return "Bool";
    }

}


/**
 * Defines AST constructor 'leq'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class leq extends Expression {
    protected Expression e1;
    protected Expression e2;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        String e1Type = e1.typeCheck(ct, tenv, cls);
        String e2Type = e2.typeCheck(ct, tenv, cls);
        if (!(e1Type.equals("Int") && e2Type.equals("Int"))) {
            ct.semantError().println("Wrong!");
        }
        set_type(TreeConstants.Bool);
        return "Bool";
    }
}


/**
 * Defines AST constructor 'comp'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class comp extends Expression {
    protected Expression e1;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        String exprType = e1.typeCheck(ct, tenv, cls);
        if (!exprType.equals("Bool")) {
            ct.semantError().println("error");
            return "Object";
        }
        set_type(TreeConstants.Bool);
        return "Bool";
    }
}


/**
 * Defines AST constructor 'int_const'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class int_const extends Expression {
    protected AbstractSymbol token;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        set_type(AbstractTable.idtable.addString("Int"));
        return "Int";
    }

    public String toString() {
        return token.toString();
    }
}


/**
 * Defines AST constructor 'bool_const'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class bool_const extends Expression {
    protected Boolean val;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        set_type(AbstractTable.idtable.addString("Bool"));
        return "Bool";
    }

    public String toString() {
        if (val)
            return "true";
        else
            return "false";
    }

}


/**
 * Defines AST constructor 'string_const'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class string_const extends Expression {
    protected AbstractSymbol token;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        set_type(AbstractTable.idtable.addString("String"));
        return "String";
    }

    public String toString() {

        return "\"" + this.token.str + "\"";
    }
}


/**
 * Defines AST constructor 'new_'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class new_ extends Expression {
    protected AbstractSymbol type_name;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        String tp = type_name.str;
        String res = null;
        if (tp.equals("SELF_TYPE")) {
            set_type(TreeConstants.SELF_TYPE);
            res = cls.name.str;
            //System.out.println("hi");
        } else if (!ct.typeExist(tp)) {
            ct.semantError().println(cls.filename + ":" + lineNumber + ": " +
                    "type " + tp + " not exist!");
            return "Object";
        } else {
            res = tp;
            set_type(AbstractTable.idtable.addString(res));
        }
        return res;
    }

}


/**
 * Defines AST constructor 'isvoid'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class isvoid extends Expression {
    protected Expression e1;

    /**
     * Creates "isvoid" AST node.
     *
     * @param lineNumber the line in the source file from which this node came.
     * @param a1         initial value for e1
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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        e1.typeCheck(ct, tenv, cls);
        set_type(TreeConstants.Bool);
        return "Bool";
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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        return null;
    }
}


/**
 * Defines AST constructor 'object'.
 * <p>
 * See <a href="TreeNode.html">TreeNode</a> for full documentation.
 */
class object extends Expression { /* Object实际上是用于存储变量的名称的! */
    protected AbstractSymbol name;

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

    public String typeCheck(ClassTable ct, SymbolTable tenv, class_c cls) {
        String res = (String) tenv.lookup(name);
        if (res == null) {
            ct.semantError().println(cls.filename.str + ":" + lineNumber + ":" +
                    "Undeclared identifier " + name + " .");
            return "Object";
        }
        if (name.str.equals("self")) {
            set_type(TreeConstants.SELF_TYPE);
        } else {
            set_type(AbstractTable.idtable.addString(res));
        }
        return res;
    }

    public String toString() {
        return name.str;
    }

}

class common {
    /* 这个类主要用于记录一些被上面的类公共使用的一些函数 */
    public static method findParentMethod(ClassTable ct, class_c cls, String methodName) {
        String parentClass = cls.parent.str; // 得到父类的名字
        class_c pCls = ct.getClass(parentClass);
        if (pCls == null) { // 如果不存在父类
            return null;
        }
        //System.out.println(cls.name.str);
        //System.out.println(className);
        Enumeration en = pCls.features.getElements();
        while (en.hasMoreElements()) {
            Object obj = en.nextElement();
            if (obj instanceof method) {
                method mtd = (method) obj;
                if (mtd.name.str.equals(methodName))
                    return mtd;
            }
        }
        return findParentMethod(ct, pCls, methodName);
    }

    public static method getMethod(ClassTable ct, class_c cls, String mtdName) {
        /**
         * 这个方法主要是为了找到对应类中名字为mtdName的方法,如果找不到的话,返回
         * null!
         */
        if (cls == null) return null;
        Enumeration en = cls.features.getElements();
        while (en.hasMoreElements()) {
            Object obj = en.nextElement();
            if (obj instanceof method) {
                method mtd = (method) obj;
                if (mtd.name.str.equals(mtdName))
                    return mtd;
            }
        }
        return getMethod(ct, ct.getClass(cls.parent.str), mtdName);
    }
 }


