import bpy
import bpy_extras
import math
import json

class MYADDON_OT_export_scene(bpy.types.Operator, bpy_extras.io_utils.ExportHelper):
    bl_idname = "myaddon.myaddon_ot_export_scene"
    bl_label = "シーン出力"
    bl_description = "シーンを出力します"
    filename_ext = ".json"

    def write_and_export(self, file, str):
        print(str)
        file.write(str)
        file.write('\n')
    
    def export(self, context):
        """ファイルに出力"""
        self.export_json(context)

    def export_json(self, context):
        """ファイルにJSON形式で出力"""

        json_object_root = dict()
        json_object_root["name"] = "scene"
        json_object_root["objects"] = list()

        print("シーン情報出力開始...%r" % self.filepath)

        #親がいるオブジェクトは親が出力するので飛ばす
        for obj in bpy.context.scene.objects:
            if(obj.parent):
                continue

            self.parse_scene_recursive(json_object_root["objects"], obj, 0)
    
        #オブジェクトをJSON文字列にエンコード
        json_text = json.dumps(json_object_root, ensure_ascii=False, cls=json.JSONEncoder, indent=4)

        #コンソールに表示
        print(json_text)

        with open(self.filepath, "wt", encoding="utf-8") as file:
            file.write(json_text)


    def parse_scene_recursive(self, data_parent, object, level):

        json_object = dict()
        json_object["type"] = object.type
        json_object["name"] = object.name

        trans,rot,scale = object.matrix_local.decompose()
        #回転をQuaternionからEulerに変換
        rot = rot.to_euler()
        rot.x = math.degrees(rot.x)
        rot.y = math.degrees(rot.y)
        rot.z = math.degrees(rot.z)

        transform = dict()
        transform["translation"] = [trans.x, trans.y, trans.z]
        transform["rotation"] = [rot.x, rot.y, rot.z]
        transform["scale"] = [scale.x, scale.y, scale.z]
        json_object["transform"] = transform

        if "file_name" in object:
            json_object["file_name"] = object["file_name"]

        if "collider" in object:
            collider = dict()
            collider["type"] = object["collider"]
            collider["center"] = list(object["collider_center"])
            collider["size"] = list(object["collider_size"])
            json_object["collider"] = collider

        data_parent.append(json_object)

        #子供リストを捜査
        if len(object.children) > 0:
            json_object["children"] = list()

            for child in object.children:
                self.parse_scene_recursive(json_object["children"], child, level + 1)

    def execute(self, context):
        
        print("シーン情報をExportします")

        self.export(context)

        print("シーン情報をExportしました")
        self.report({'INFO'}, "シーン情報をExportしました")

        return {'FINISHED'}
