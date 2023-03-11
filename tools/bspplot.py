#!/usr/bin/env python3
#
# Copyright(C) 2023 Brian Koropoff
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# DESCRIPTION:
# 	BSP Analysis
#

from abc import ABC
import itertools
import json
from math import atan2, cos, pi, sin, sqrt
import os
import sys

from matplotlib import collections as mc, pyplot as plt


class Container(ABC):
    def __init__(self, id, data, root):
        self._id = id
        self._data = data
        self._root = root

    @property
    def id(self):
        return self._id

    def _wrap_elem(self, k, v):
        if hasattr(type(self), "_element_type"):
            return getattr(type(self), "_element_type")(k, v, self._root)
        elif isinstance(v, dict):
            return Object(k, v, self._root)
        elif isinstance(v, list):
            return Array(k, v, self._root)
        else:
            return v

    def __eq__(self, right):
        if isinstance(right, list):
            return any(self == e for e in right)
        return self.id == right


class Object(Container):
    def __init__(self, id, data, root):
        super().__init__(id, data, root)
        self._keys = {
            *data.keys(),
            *(
                k
                for k, v in type(self).__dict__.items()
                if not k.startswith("_") and hasattr(v, "__get__")
            ),
        }

    def __init_subclass__(cls):
        for k, v in getattr(cls, "__annotations__", {}).items():
            if k.startswith("_"):
                continue
            if issubclass(v, Container) and v not in {Object, Array}:
                wrap = Wrap(v)
                wrap.__set_name__(cls, k)
                setattr(cls, k, wrap)

    def __getattr__(self, k):
        return self._wrap_elem(k, self._data[k])

    def __getitem__(self, k):
        try:
            return getattr(self, k)
        except AttributeError:
            raise KeyError(k)

    def __contains__(self, k):
        return k in self._keys

    def __iter__(self):
        return iter(self._keys)

    def keys(self):
        return iter(self._keys)

    def values(self):
        return (self[k] for k in self._keys)

    def items(self):
        return ((k, self[k]) for k in self._keys)

    def get(self, k, default=None):
        try:
            return self[k]
        except KeyError:
            return default

    def __repr__(self):
        inner = ", ".join(
            f"{repr(k)}={repr(v) if not isinstance(v, Container) else '...'}"
            for k, v in self.items()
        )
        return f"{type(self).__name__}({inner})"

    def __bool__(self):
        return bool(self._data)

    def __len__(self):
        return len(self._data)


class Array(Container):
    def __getitem__(self, k):
        if isinstance(k, str):
            return Array(k, [e[k] for e in self], self._root)
        else:
            return self._wrap_elem(k, self._data[k])

    def __getattr__(self, k):
        return self[k]

    def __iter__(self):
        return (self[k] for k in range(len(self._data)))

    def __eq__(self, right):
        if isinstance(right, list):
            return any(self == e for e in right)
        return any(right == i for i in self)

    def __repr__(self):
        inner = ", ".join(
            repr(v) if not isinstance(v, Container) else "..." for v in self
        )
        return f"[{inner}]"


class Wrap(object):
    def __init__(self, cls):
        self._cls = cls

    def __set_name__(self, _, name):
        self._name = name

    def __get__(self, instance, owner=None):
        return self._cls(self._name, instance._data[self._name], instance._root)


class Lookup(object):
    def __init__(self, section):
        self._section = section

    def __set_name__(self, _, name):
        self._name = name

    def __get__(self, instance, _=None):
        return instance._root[self._section][instance._data[self._name]]


def ObjectWithValues(type):
    class Container(Object):
        _element_type = type

    return Wrap(Container)


def ArrayWithValues(type):
    class Container(Array):
        _element_type = type

    return Wrap(Container)


def LookupArray(section):
    class Inner(Array):
        def _wrap_elem(self, _, v):
            return self._root[section][v]

    return Wrap(Inner)


class Sector(Object):
    lines = LookupArray("lines")


class Subsector(Object):
    sector = Lookup("sectors")
    segs = LookupArray("segs")
    isegs = LookupArray("isegs")


class Line(Object):
    front = Lookup("sides")
    back = Lookup("sides")


class Side(Object):
    sector = Lookup("sectors")


class Seg(Object):
    sector = Lookup("sectors")
    subsector = Lookup("subsectors")
    adjacent = LookupArray("subsectors")
    v1 = Lookup("vertices")
    v2 = Lookup("vertices")

    @property
    def coords(self):
        return ((self.v1.x, self.v1.y), (self.v2.x, self.v2.y))


class ISeg(Object):
    sector = Lookup("sectors")
    subsector = Lookup("subsectors")
    adjacent = LookupArray("subsectors")
    node = Lookup("nodes")

    @property
    def coords(self):
        return ((self.start.x, self.start.y), (self.end.x, self.end.y))


class Children(Object):
    def _node_or_subsector(self, data):
        if "node" in data:
            return self._root.nodes[data["node"]]
        else:
            return self._root.subsectors[data["subsector"]]

    @property
    def right(self):
        return self._node_or_subsector(self._data["right"])

    @property
    def left(self):
        return self._node_or_subsector(self._data["left"])


class Node(Object):
    children: Children
    isegs = LookupArray("isegs")
    _subsectors = None
    _sectors = None

    @property
    def coords(self):
        dv = self.partition
        return ((dv.start.x, dv.start.y), (dv.end.x, dv.end.y))

    def _subsector(self):
        for child in self.children.values():
            if isinstance(child, Subsector):
                yield child
            else:
                yield from child.subsector

    @property
    def subsector(self):
        if self._subsectors is None:
            self._subsectors = Array("subsector", list(self._subsector()), self._root)
        return self._subsectors

    @property
    def sector(self):
        if self._sectors is None:
            self._sectors = Array(
                "subsector", [s.sector for s in self.subsector], self._root
            )
        return self._sectors


class BSP(Object):
    _empty = {
        "vertices": {},
        "lines": {},
        "sides": {},
        "segs": {},
        "isegs": {},
        "sectors": {},
        "subsectors": {},
        "nodes": {},
    }

    def __init__(self, data=None):
        if data is None:
            data = self._empty
        super().__init__(None, data, self)

    lines = ObjectWithValues(Line)
    sides = ObjectWithValues(Side)
    segs = ObjectWithValues(Seg)
    isegs = ObjectWithValues(ISeg)
    sectors = ObjectWithValues(Sector)
    subsectors = ObjectWithValues(Subsector)
    nodes = ObjectWithValues(Node)


class Plotter(object):
    tick_length_ratio = 1 / 10
    tick_width_ratio = 1 / 2

    def __init__(self):
        self._bsp = BSP()
        self._title = None
        self.clear()

    def _addoffset(self, coords):
        return tuple((c[0] + self._offset[0], c[1] + self._offset[1]) for c in coords)

    def _addsimpleline(self, coords):
        self._lines.append(self._addoffset(coords))
        self._colors.append(self._color)
        self._widths.append(self._width)

    def _addtick(self, coords, annotation=None):
        dy = coords[1][1] - coords[0][1]
        dx = coords[1][0] - coords[0][0]
        ang = atan2(dy, dx) - pi / 2
        mid = ((coords[0][0] + coords[1][0]) / 2, (coords[0][1] + coords[1][1]) / 2)
        norm = sqrt(dx * dx + dy * dy) * self.tick_length_ratio
        end = (mid[0] + norm * cos(ang), mid[1] + norm * sin(ang))
        tick = (mid, end)

        self._lines.append(self._addoffset(tick))
        self._colors.append(self._color)
        self._widths.append(self._width * self.tick_width_ratio)

        if annotation:
            self._addlineannotation(end, annotation)

    def _addlineannotation(self, coords, text):
        if self._annotate:
            self._annotations.append((coords, text))

    def _addline(self, coords, tick=False, annotation=None):
        self._addsimpleline(coords)
        if tick:
            self._addtick(coords, annotation)

    def _drawxsegs(self, obj, kind):
        for i, s in obj.items():
            if self._select(i, s):
                self._addline(s.coords, tick=True, annotation=f"{kind}{i}")

    def segs(self):
        self._drawxsegs(self._bsp.segs, kind="seg")

    def isegs(self):
        self._drawxsegs(self._bsp.isegs, kind="iseg")

    def xsegs(self):
        self.isegs()
        self.segs()

    def partitions(self):
        for i, n in self._bsp.nodes.items():
            if self._select(i, n):
                self._addline(n.coords, tick=True, annotation=f"div{i}")

    def reset(self):
        self._select = lambda _k, _v: True
        self._color = (0, 0, 0, 1)
        self._width = 1
        self._annotate = False
        self._offset = (0, 0)

    def clear(self):
        self.reset()
        self._lines = []
        self._colors = []
        self._widths = []
        self._annotations = []

    def load(self, path):
        with open(path, "r") as h:
            self._bsp = BSP(json.load(h))
        if self._title is None:
            self._title = os.path.basename(path)

    def color(self, color):
        self._color = color

    def width(self, width):
        self._width = width

    def offset(self, xy):
        self._offset = xy

    def annotate(self, annotate):
        self._annotate = annotate

    def select(self, f):
        self._select = f

    def plot(self):
        lc = mc.LineCollection(
            self._lines, colors=self._colors, linewidths=self._widths
        )
        fig, ax = plt.subplots()
        fig.canvas.manager.set_window_title(self._title)
        ax.add_collection(lc)
        ax.autoscale()
        ax.set_aspect("equal")
        ax.margins(0.1)
        for p, text in self._annotations:
            plt.annotate(text, xy=p, fontsize=5)
        plt.show()


class Expr(object):
    _ops = {
        "=": lambda l, r: l == r,
        "==": lambda l, r: l == r,
        "!=": lambda l, r: l != r,
        ">": lambda l, r: l > r,
        ">=": lambda l, r: l >= r,
        "<": lambda l, r: l < r,
        "<=": lambda l, r: l <= r,
        "__true__": lambda l, r: bool(l) == r,
    }

    def __init__(self, expr, filter=None):
        self._expr = expr
        self._filter = filter
        self._dnf = self._parse(expr)

    @staticmethod
    def _parse_term(term):
        parts = term.split()
        if len(parts) == 3:
            var, op, val = parts
            if "," in val:
                val = val.split(",")
        else:
            if term.startswith("!"):
                var, op, val = (term[1:], "__true__", False)
            else:
                var, op, val = (term, "__true__", True)
        return (var, op, val)

    @staticmethod
    def _fetch(var, v):
        parts = var.split(".")
        for part in parts:
            v = v[part]
        return v

    @classmethod
    def _fetch_eval(cls, op, var, v, right):
        try:
            left = cls._fetch(var, v)
        except KeyError:
            print(f"Warning: no such key {var} in {type(v).__name__}")
            return False
        if not isinstance(left, Container) and isinstance(right, list):
            return any(left == r for r in right)
        return op(left, right)

    @classmethod
    def _term(cls, var, op, val):
        opf = cls._ops[op]
        return lambda _, v: cls._fetch_eval(opf, var, v, val)

    def _parse(self, expr):
        terms = (
            [*(self._parse_term(t) for t in conj.split(" and "))]
            for conj in expr.split(" or ")
        )
        return [[self._term(*t) for t in conj] for conj in terms]

    def __call__(self, k, v):
        filter = self._filter or (lambda _k, _v: True)
        return filter(k, v) and any(all(t(k, v) for t in conj) for conj in self._dnf)

    def __repr__(self):
        if self._filter:
            return f"Expr({repr(self._expr)}, filter={repr(self._filter)})"
        else:
            return f"Expr({repr(self._expr)})"


class Parser(object):
    def __init__(self, args):
        self._args = list(reversed(args))
        self._options = []
        self._commands = []
        self._filter = None
        self._datapath = None
        self._no_select = True
        self._directive = None

    def _pop(self, name):
        if not len(self._args):
            print("Missing argument:", name)
            sys.exit(1)
        return self._args.pop()

    def _push(self, method, *args, **kwargs):
        self._options.append((method, args, kwargs))

    def _flush(self):
        if self._directive:
            if self._filter and self._no_select:
                self._push("select", self._filter)
            self._commands.extend(self._options)
            self._commands.append(self._directive)
            self._directive = None
            self._options = []
            self._push("reset")
            self._no_select = True

    def _start(self, command, *args, **kwargs):
        self._flush()
        self._directive = (command, args, kwargs)

    def parse(self):
        while len(self._args):
            arg = self._pop("")
            if arg == "--color":
                c = self._pop("color")
                color = tuple(float(x) for x in c.split(","))
                if len(color) == 3:
                    color = color + (1,)
                self._push("color", color)
                continue

            if arg == "--width":
                width = float(self._pop("width"))
                self._push("width", width)
                continue

            if arg == "--offset":
                xy = tuple(float(v) for v in self._pop("width").split(","))
                self._push("offset", xy)
                continue

            if arg == "--annotate":
                self._push("annotate", True)
                continue

            if arg == "--filter":
                expr = self._pop("expr")
                self._filter = Expr(expr)
                continue

            if arg == "--select":
                expr = self._pop("expr")
                self._push("select", Expr(expr, self._filter))
                self._no_select = False
                continue

            if arg == "segs":
                self._start("segs")
                continue

            if arg == "isegs":
                self._start("isegs")
                continue

            if arg == "xsegs":
                self._start("xsegs")
                continue

            if arg == "partitions":
                self._start("partitions")
                continue

            if arg.startswith("-"):
                print("Unrecognized option:", arg)
                sys.exit(1)

            if self._datapath is None:
                if not os.path.isfile(arg):
                    print("No such file:", arg)
                    sys.exit(1)
                self._datapath = arg
            else:
                print("Extraneous argument:", arg)
                sys.exit(1)

        if self._datapath is None:
            print("No data file specified")
            sys.exit(1)

        self._flush()
        self._commands.insert(0, ("load", [self._datapath], {}))
        self._commands.append(("plot", [], {}))
        return self._commands


def main():
    plotter = Plotter()

    print("from bspplot import Plotter, Expr\n")
    print("p = Plotter()")

    for command, args, kwargs in Parser(sys.argv[1:]).parse():
        argstr = ", ".join(repr(p) for p in args)
        kwargstr = ", ".join(f"{k}={repr(v)}" for k, v in kwargs.items())
        print(f"p.{command}({argstr}{', ' if argstr and kwargstr else ''}{kwargstr})")
        getattr(plotter, command)(*args, **kwargs)


if __name__ == "__main__":
    main()
