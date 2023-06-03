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
import json
from math import atan2, cos, pi, sin, sqrt
import os
import sys

from matplotlib import collections as mc, pyplot as plt
from matplotlib.path import Path


class Container(ABC):
    _props = set()

    def __init__(self, id, data, root):
        self._id = id
        self._data = data
        self._root = root
        self._cache = {}

    def __init_subclass__(cls):
        cls._props = {
            k
            for k, v in cls.__dict__.items()
            if not k.startswith("_") and hasattr(v, "__get__")
        }

    @property
    def id(self):
        return self._id

    def __len__(self):
        return len(self._data)

    def __getattribute__(self, k):
        if k.startswith("_"):
            return super().__getattribute__(k)
        cache = self._cache
        if k in cache:
            return cache[k]
        try:
            v = super().__getattribute__(k)
        except AttributeError:
            v = self.__getattr__(k)  # type: ignore
        v = self._wrap_elem(k, v)
        cache[k] = v
        return v

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

    def _shallow_repr(self):
        return f"{type(self).__name__}(id={repr(self.id)})"


def root(cont):
    return cont._root


class Object(Container):
    def __init__(self, id, data, root):
        super().__init__(id, data, root)
        self._attrs = {*self._props, *data.keys()}

    def __init_subclass__(cls):
        for k, v in getattr(cls, "__annotations__", {}).items():
            if k.startswith("_"):
                continue
            if issubclass(v, Container) and v not in {Object, Array}:
                wrap = Wrap(v)
                wrap.__set_name__(cls, k)
                setattr(cls, k, wrap)
        super().__init_subclass__()

    def __getattr__(self, k):
        return self._data[k]

    def __getitem__(self, k):
        try:
            return getattr(self, k)
        except AttributeError:
            raise KeyError(k)

    def __contains__(self, k):
        return k in self._attrs

    def __iter__(self):
        return iter(self._attrs)

    def __repr__(self):
        inner = ", ".join(
            f"{k}={repr(v) if not isinstance(v, Container) else v._shallow_repr()}"
            for k, v in sorted(items(self))
        )
        return f"{type(self).__name__}({inner})"

    def __bool__(self):
        return bool(self._data)

    def __len__(self):
        return len(self._data)


def items(obj):
    return ((k, obj[k]) for k in obj)


def keys(obj):
    return iter(obj)


def values(obj):
    return (obj[k] for k in obj)


class NullObject(Object):
    def __bool__(self):
        return False

    def __len__(self):
        return 0

    def __iter__(self):
        return iter([])

    def __getattr__(self, _):
        return None


null = NullObject(None, {}, None)


class Array(Container):
    def __init__(self, id, data, root):
        super().__init__(id, data, root)

    def __getitem__(self, k):
        if isinstance(k, str):
            try:
                return getattr(self, k)
            except AttributeError:
                raise KeyError(k)

        cache = self._cache
        if k in cache:
            return cache[k]
        v = self._wrap_elem(k, self._data[k])
        cache[k] = v
        return v

    def __getattr__(self, k):
        return [getattr(e, k) for e in self]

    def __iter__(self):
        return (self[k] for k in range(len(self._data)))

    def __eq__(self, right):
        if isinstance(right, list):
            return any(self == e for e in right)
        return any(right == i for i in self)

    def __bool__(self):
        return any(bool(e) for e in self)

    def __repr__(self):
        inner = ", ".join(
            repr(v) if not isinstance(v, Container) else "..." for v in self
        )
        return f"Array([{inner}])"


class Wrap(object):
    def __init__(self, cls):
        self._cls = cls

    def __set_name__(self, _, name):
        self._name = name

    def __get__(self, instance, owner=None):
        name = self._name
        return self._cls(name, instance._data[name], instance._root)


class Lookup(object):
    def __init__(self, section):
        self._section = section

    def __set_name__(self, _, name):
        self._name = name

    def __get__(self, instance, _=None):
        id = instance._data[self._name]
        if id is None:
            return null
        section = instance._root[self._section]
        return section[id]


def ObjectWithValues(type):
    class InnerObject(Object):
        _element_type = type

    return Wrap(InnerObject)


def ArrayWithValues(type):
    class InnerArray(Array):
        _element_type = type

    return Wrap(InnerArray)


def LookupArray(section):
    class InnerLookupArray(Array):
        def _wrap_elem(self, k, v):
            if k is None:
                return null
            if isinstance(k, int):
                return self._root[section][v]
            return super()._wrap_elem(k, v)

    return Wrap(InnerLookupArray)


class Sector(Object):
    lines = LookupArray("lines")


class Subsector(Object):
    sector = Lookup("sectors")
    chunk = Lookup("chunks")
    segs = LookupArray("segs")

    def coords(self):
        return [(s.v1.x, s.v1.y) for s in self.segs]

class Line(Object):
    v1 = Lookup("vertices")
    v2 = Lookup("vertices")
    front = Lookup("sides")
    back = Lookup("sides")

    @property
    def sector(self):
        front = self.front
        back = self.back
        return [*([front.sector] if front else []), *([back.sector] if back else [])]

    def coords(self):
        return ((self.v1.x, self.v1.y), (self.v2.x, self.v2.y))


class Side(Object):
    sector = Lookup("sectors")


class Seg(Object):
    subsector = Lookup("subsectors")
    partner = Lookup("segs")
    line = Lookup("lines")
    side = Lookup("sides")
    v1 = Lookup("vertices")
    v2 = Lookup("vertices")

    @property
    def chunk(self):
        return self.subsector.chunk

    @property
    def sector(self):
        return self.subsector.sector

    def coords(self):
        return ((self.v1.x, self.v1.y), (self.v2.x, self.v2.y))


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

    def coords(self):
        dv = self.partition
        return ((dv.start.x, dv.start.y), (dv.end.x, dv.end.y))

    def _subsector(self):
        for k in self.children:
            child = self.children[k]
            if isinstance(child, Subsector):
                yield child
            else:
                yield from child.subsector

    @property
    def subsector(self):
        return list(self._subsector())

    @property
    def chunk(self):
        return [s.chunk for s in self.subsector]

    @property
    def sector(self):
        return [s.sector for s in self.subsector]


class Chunk(Object):
    sector = Lookup("sectors")
    subsectors = LookupArray("subsectors")
    perimeter = LookupArray("segs")
    path = LookupArray("paths")
    bleeds = LookupArray("bleeds")

    @property
    def in_bleeds(self):
        return root(self)._in_bleeds_for(self.id)

    def coords(self):
        run = []
        for p in self.path:
            if p is None:
                if run:
                    yield run
                run = []
                continue
            run.append((p.x, p.y))
        if run:
            yield run


class Bleed(Object):
    target = Lookup("chunks")


class Portal(Object):
    chunk = Lookup("chunks")

    def coords(self):
        return [
            [
                (self.bbox.left, self.bbox.top),
                (self.bbox.left, self.bbox.bottom),
                (self.bbox.right, self.bbox.bottom),
                (self.bbox.right, self.bbox.top),
            ]
        ]


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
        "chunks": {},
        "paths": {},
        "bleeds": {},
    }

    _in_bleeds = {}

    def _compute_in_bleeds(self):
        result = {}
        for chunk in values(self.chunks):
            for bleed in chunk.bleeds:
                tid = bleed.target.id
                if tid in result:
                    l = result[tid]
                else:
                    l = []
                    result[tid] = l
                l.append(bleed)
        self._in_bleeds = result

    def _in_bleeds_for(self, id):
        if not self._in_bleeds:
            self._compute_in_bleeds()
        return self._in_bleeds.get(id, [])

    def __init__(self, data=None):
        if data is None:
            data = self._empty
        super().__init__(None, data, self)

    lines = ObjectWithValues(Line)
    sides = ObjectWithValues(Side)
    segs = ObjectWithValues(Seg)
    sectors = ObjectWithValues(Sector)
    subsectors = ObjectWithValues(Subsector)
    nodes = ObjectWithValues(Node)
    chunks = ObjectWithValues(Chunk)
    bleeds = ObjectWithValues(Bleed)


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
        self._linecolors.append(self._edgecolor)
        self._linewidths.append(self._width)

    def _addpolypath(self, coords, annotation=None):
        codes = []
        verts = []
        for cont in coords:
            codes.append(Path.MOVETO)
            codes.extend([Path.LINETO] * (len(cont) - 1))
            verts.extend(self._addoffset(cont))
            codes.append(Path.CLOSEPOLY)
            verts.append((0, 0))

        if not len(codes):
            return

        path = Path(verts, codes)
        self._polys.append(path)
        self._polywidths.append(self._width)
        self._polyfacecolors.append(self._facecolor)
        self._polyedgecolors.append(self._edgecolor)

        if annotation:
            extent = path.get_extents()
            coords = ((extent.x0 + extent.x1) / 2, (extent.y0 + extent.y1) / 2)
            self._addannotation(coords, annotation)

    def _addtick(self, coords, annotation=None):
        dy = coords[1][1] - coords[0][1]
        dx = coords[1][0] - coords[0][0]
        ang = atan2(dy, dx) - pi / 2
        mid = ((coords[0][0] + coords[1][0]) / 2, (coords[0][1] + coords[1][1]) / 2)
        norm = sqrt(dx * dx + dy * dy) * self.tick_length_ratio
        end = (mid[0] + norm * cos(ang), mid[1] + norm * sin(ang))
        tick = (mid, end)

        self._lines.append(self._addoffset(tick))
        self._linecolors.append(self._edgecolor)
        self._linewidths.append(self._width * self.tick_width_ratio)

        if annotation:
            self._addannotation(end, annotation)

    def _addannotation(self, coords, text):
        if self._annotate:
            self._annotations.append((coords, text))

    def _addline(self, coords, tick=False, annotation=None):
        self._addsimpleline(coords)
        if tick:
            self._addtick(coords, annotation)

    def lines(self):
        for i in self._bsp.lines:
            l = self._bsp.lines[i]
            if self._select(i, l):
                self._addline(l.coords(), tick=True, annotation=f"line{i}")

    def sides(self):
        for i in self._bsp.lines:
            l = self._bsp.lines[i]
            for j, s in enumerate([l.front, l.back]):
                if s is not None and self._select(j, s):
                    coords = list(l.coords())
                    if j == 1:
                        coords.reverse()
                    self._addline(coords, tick=True, annotation=f"side{i}")

    def segs(self):
        for i in self._bsp.segs:
            s = self._bsp.segs[i]
            if self._select(i, s):
                self._addline(s.coords(), tick=True, annotation=f"seg{i}")

    def partitions(self):
        for i in self._bsp.nodes:
            n = self._bsp.nodes[i]
            if self._select(i, n):
                self._addline(n.coords(), tick=True, annotation=f"part{i}")

    def paths(self):
        for i in self._bsp.chunks:
            c = self._bsp.chunks[i]
            if self._select(i, c):
                self._addpolypath(c.coords(), annotation=f"chunk{i}")

    def triangles(self):
        for i in self._bsp.chunks:
            c = self._bsp.chunks[i]
            if self._select(i, c):
                for t in c.triangles:
                    self._addpolypath([[(t[0].x, t[0].y),(t[1].x, t[1].y),(t[2].x, t[2].y)]]);

    def subsectors(self):
        for i, s in items(self._bsp.subsectors):
            if self._select(i, s):
                self._addpolypath([s.coords()], annotation=f"ssec{i}")

    def reset(self):
        self._select = lambda _k, _v: True
        self._facecolor = (0.5, 0.5, 0.5, 1)
        self._edgecolor = (0, 0, 0, 1)
        self._width = 1
        self._annotate = False
        self._offset = (0, 0)

    def clear(self):
        self.reset()
        self._lines = []
        self._linecolors = []
        self._linewidths = []
        self._polys = []
        self._polywidths = []
        self._polyfacecolors = []
        self._polyedgecolors = []
        self._annotations = []
        self._zorder = 0

    def load(self, path):
        with open(path, "r") as h:
            self._bsp = BSP(json.load(h))
        if self._title is None:
            self._title = os.path.basename(path)

    def facecolor(self, color):
        self._facecolor = color

    def edgecolor(self, color):
        self._edgecolor = color

    def color(self, color):
        self._facecolor = color
        self._edgecolor = color

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
            self._lines,
            colors=self._linecolors,
            linewidths=self._linewidths,
            capstyle="round",
            joinstyle="round",
        )
        pc = mc.PathCollection(
            self._polys,
            linewidths=self._polywidths,
            facecolors=self._polyfacecolors,
            edgecolors=self._polyedgecolors,
            zorder=2,
        )
        fig, ax = plt.subplots()
        fig.canvas.manager.set_window_title(self._title)
        ax.add_collection(lc)
        ax.add_collection(pc)
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
        except KeyError as e:
            print(f"Warning: no such key {var} in {type(v).__name__}: {e}")
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

            if arg == "--facecolor":
                c = self._pop("color")
                color = tuple(float(x) for x in c.split(","))
                if len(color) == 3:
                    color = color + (1,)
                self._push("facecolor", color)
                continue

            if arg == "--edgecolor":
                c = self._pop("color")
                color = tuple(float(x) for x in c.split(","))
                if len(color) == 3:
                    color = color + (1,)
                self._push("edgecolor", color)
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

            if arg == "lines":
                self._start("lines")
                continue

            if arg == "sides":
                self._start("sides")
                continue

            if arg == "segs":
                self._start("segs")
                continue

            if arg == "subsectors":
                self._start("subsectors")
                continue

            if arg == "partitions":
                self._start("partitions")
                continue

            if arg == "paths":
                self._start("paths")
                continue

            if arg == "triangles":
                self._start("triangles")
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
