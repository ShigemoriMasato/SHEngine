import bpy
import gpu
import gpu_extras.batch
import copy
import mathutils

class DrawCollider:

    handle = None

    def draw_collider():
        #頂点
        vertices = {"pos": []}
        #インデックス
        indices = []

        offsets = [
            [-0.5,-0.5,-0.5],
            [+0.5,-0.5,-0.5],
            [-0.5,+0.5,-0.5],
            [+0.5,+0.5,-0.5],
            [-0.5,-0.5,+0.5],
            [+0.5,-0.5,+0.5],
            [-0.5,+0.5,+0.5],
            [+0.5,+0.5,+0.5]
        ]

        for object in bpy.context.scene.objects:

            if "collider" not in object:
                continue

            center = mathutils.Vector(object["collider_center"])
            size = mathutils.Vector(object["collider_size"])

            start = len(vertices["pos"])

            for offset in offsets:
                pos = copy.copy(center)
                pos.x += offset[0] * size[0]
                pos.y += offset[1] * size[1]
                pos.z += offset[2] * size[2]
                pos = object.matrix_world @ pos
                vertices["pos"].append(pos)
                
                indices.append((start + 0, start + 1))
                indices.append((start + 2, start + 3))
                indices.append((start + 0, start + 2))
                indices.append((start + 1, start + 3))

                indices.append((start + 4, start + 5))
                indices.append((start + 6, start + 7))
                indices.append((start + 4, start + 6))
                indices.append((start + 5, start + 7))

                indices.append((start + 0, start + 4))
                indices.append((start + 1, start + 5))
                indices.append((start + 2, start + 6))
                indices.append((start + 3, start + 7))

            

        #ビルトインのシェーダーを取得
        shader = gpu.shader.from_builtin('UNIFORM_COLOR')
        
        #バッチを作成(引数:シェーダー、トポロジー、頂点データ、インデックスデータ)
        batch = gpu_extras.batch.batch_for_shader(shader, 'LINES', vertices, indices=indices)

        #シェーダーのパラメータの設定
        color=[0.5, 1.0, 1.0, 1.0]
        shader.bind()
        shader.uniform_float("color", color)
        batch.draw(shader)

