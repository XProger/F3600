import bpy
import struct
from mathutils import Vector
from mathutils import Matrix
from math import radians

TypeF3      = 1
TypeFT3     = 2
TypeF4      = 3
TypeFT4     = 4
TypeG3      = 5
TypeGT3     = 6
TypeG4      = 7
TypeGT4     = 8
TypeTSPR    = 10
TypeBSPR    = 11

SCALE = 0.001

file = None

def skip(count):
    file.seek(count, 1)

def read_str():
    bytes = struct.unpack('16s', file.read(16))[0]
    for i in range(len(bytes)):
        if bytes[i] == 0:
             return str(bytes[0:i], 'ansi')
    return str(bytes, 'ansi')

def read_int16():
    return struct.unpack('>h', file.read(2))[0]

def read_uint16():
    return struct.unpack('>H', file.read(2))[0]

def read_int32():
    return struct.unpack('>i', file.read(4))[0]

def read_skeleton():
    skip(9 * 2) # relative rot (identity)
    skip(2) # padding
    skip(3 * 4) # relative pos == absolute pos

    skip(9 * 2) # absolute rot (identity)
    skip(2) # padding
    pos = struct.unpack('>3i', file.read(3 * 4)) # absolute pos
    skip(2) # update
    skip(2) # padding
    skip(4) # super
    skip(4) # sub
    skip(4) # next

    print(pos)
    return pos

def read_vertices(count):
    ret = []
    for i in range(count):
        v = struct.unpack('>3h', file.read(6))
        skip(2) # padding
        ret.append(v)
    return ret

def read_coords(count):
    ret = []
    for i in range(count):
        v = struct.unpack('>H', file.read(2))[0]
        ret.append(v)
    return ret

def read_colors(count):
    ret = []
    for i in range(count):
        v = struct.unpack('4B', file.read(4))[0]
        ret.append(v)
    return ret

def read_uvs(count):
    ret = []
    for i in range(count):
        v = struct.unpack('2B', file.read(2))[0]
        ret.append(v)
    return ret

def read_f3(p):
    p.coords = read_coords(3)
    skip(2) # padding
    p.colors = read_colors(1)
    return

def read_ft3(p):
    p.coords = read_coords(3)
    p.texture = read_uint16()
    skip(2) # cba
    skip(2) # tsb
    p.uvs = read_uvs(3)
    skip(2) # padding
    p.colors = read_colors(1)
    return

def read_f4(p):
    p.coords = read_coords(4)
    p.colors = read_colors(1)
    return

def read_ft4(p):
    p.coords = read_coords(4)
    p.texture = read_uint16()
    skip(2) # cba
    skip(2) # tsb
    p.uvs = read_uvs(4)
    skip(2) # padding
    p.colors = read_colors(1)
    return

def read_g3(p):
    p.coords = read_coords(3)
    skip(2) # padding
    p.colors = read_colors(3)
    return

def read_gt3(p):
    p.coords = read_coords(3)
    p.texture = read_uint16()
    skip(2) # cba
    skip(2) # tsb
    p.uvs = read_uvs(3)
    skip(2) # padding
    p.colors = read_colors(3)
    return

def read_g4(p):
    p.coords = read_coords(4)
    p.colors = read_colors(4)
    return

def read_gt4(p):
    p.coords = read_coords(4)
    p.texture = read_uint16()
    skip(2) # cba
    skip(2) # tsb
    p.uvs = read_uvs(4)
    skip(2) # padding
    p.colors = read_colors(4)
    return

def read_spr(p):
    p.coords = read_coords(1)
    p.width = read_int16()
    p.height = read_int16()
    p.texture = read_uint16()
    p.colors = read_colors(1)
    return

def read_primitives(count):
    ret = []
    for i in range(count):
        p = type('', (), {})()
        p.type = read_uint16()
        p.flag = read_uint16()
        if p.type == TypeF3:
            read_f3(p)
        if p.type == TypeFT3:
            read_ft3(p)
        if p.type == TypeF4:
            read_f4(p)
        if p.type == TypeFT4:
            read_ft4(p)
        if p.type == TypeG3:
            read_g3(p)
        if p.type == TypeGT3:
            read_gt3(p)
        if p.type == TypeG4:
            read_g4(p)
        if p.type == TypeGT4:
            read_gt4(p)
        if p.type == TypeTSPR or p.type == TypeBSPR:
            read_spr(p)
        ret.append(p)
    return ret

def read_obj(objIdx):
    name = str(objIdx) + "_" + read_str()

    vertexCount = read_uint16()
    skip(2) # padding
    skip(4) # vertices

    skip(2) # normalCount
    skip(2) # padding
    skip(4) # normals

    primitiveCount = read_uint16()
    skip(2) # padding
    skip(4) # primitives

    skip(4 * 2) # unused
    skip(4) # skeleton
    extent = read_int32()
    skip(2) # unused
    skip(2) # padding
    skip(4) # next
    pos = read_skeleton()

    vertices = read_vertices(vertexCount)
    primitives = read_primitives(primitiveCount)

    print(name)
    print(primitiveCount)

    meshVertices = []
    meshFaces = []

    for i in range(vertexCount):
        meshVertices.append(vertices[i])

    for i in range(primitiveCount):
        idx = primitives[i].coords
        if len(idx) == 1:
            continue
        if len(idx) == 4:
            idx[2], idx[3] = idx[3], idx[2]
        meshFaces.append(idx)

    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(meshVertices, [], meshFaces)
    mesh.update()

    obj = bpy.data.objects.new(name, mesh)
    mat_scale = Matrix.Diagonal(Vector([SCALE, SCALE, SCALE, 1]))
    mat_trans = Matrix.Translation(Vector([pos[0], pos[1], pos[2], 0]))
    mat_rot = Matrix.Rotation(radians(90.0), 4, 'X')
    obj.matrix_local = mat_rot @ mat_scale @ mat_trans
    return obj

def read_scene(index):
    path = 'C:\\wipeout-xport\\bin\\WIPEOUT\\TRACK{:02d}\\SCENE.PRM'.format(index)
    #path = 'C:\\Projects\\wipeout-xport\\bin\\WIPEOUT\\COMMON\\ALLSH.PRM'.format(index)
    global file;
    file = open(path, 'rb')
    file.seek(0, 2)
    size = file.tell()
    file.seek(0)

    objIdx = 0

    while file.tell() < size:
        obj = read_obj(objIdx)
        bpy.context.collection.objects.link(obj)
        objIdx += 1
        
    file.close()
    
def write_shadow_mesh(file, mesh):
    file.write(struct.pack('>i', len(mesh.vertices)))
    file.write(struct.pack('>i', len(mesh.polygons)))

    for v in mesh.vertices:
        v.co.x = round(v.co.x / 16) * 16
        v.co.z = round(v.co.z / 16) * 16
        # write coords shifted down by 4 to improve MUL perf
        file.write(struct.pack('>h', round(v.co.x / 16)))
        file.write(struct.pack('>h', round(v.co.z / 16)))
        
    for f in mesh.polygons:
        for i in f.loop_indices:
            idx = mesh.loops[i].vertex_index
            file.write(struct.pack('B', idx))
        # duplicate the last index for tri to make quad
        if len(f.loop_indices) == 3:
            idx = mesh.loops[2].vertex_index
            file.write(struct.pack('B', idx))
        # check for tri and quads
        if len(f.loop_indices) > 4:
            printf("too much indices for " + mesh.name)


def write_shadows(path):
    global file;
    file = open(path, 'wb')
    
    objs = bpy.context.selected_objects
    objs.sort(key = lambda o: o.name)
    print(str(len(objs)) + " shadows")
    
    for obj in objs:
        write_shadow_mesh(file, obj.data)
    
    file.close()

#read_scene(2)

write_shadows('C:\\Projects\\wipeout-xport\\src\\platform\\3do\\CD\\WIPEOUT\\COMMON\\allsh.shd')

