import bpy

from .test_class import MYADDON_OT_stretch_vertex, MYADDON_OT_create_ico_sphere
from .export_scene import MYADDON_OT_export_scene
from .topbar import TOPBAR_MT_my_menu
from .draw_collider import DrawCollider
from .collider import MYADDON_OT_add_collider
from .collider_pt import OBJECT_PT_collider

#ブレンダーに登録するアドオン情報
bl_info = {
    "name": "レベルエディタ",
    "author": "Shigemori",
    "version": (1, 0),
    "blender": (3, 3, 1),
    "location": "",
    "description":"レベルエディタ",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object"
}



class OBJECT_PT_file_name(bpy.types.Panel):
    """オブジェクトのファイルネームパネル"""
    bl_idname = "OBJECT_PT_file_name"
    bl_label = "ファイルネーム"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    def draw(self, context):
        #パネルに項目を追加
        if "file_name" in context.object:
            self.layout.prop(context.object, '["file_name"]', text=self.bl_label)

        else:
            self.layout.operator(MYADDON_OT_add_filename.bl_idname, text="FileName追加", icon='ADD')

#オペレーター　カスタムプロパティ[filename]を追加
class MYADDON_OT_add_filename(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_ot_add_filename"
    bl_label = "FileName追加"
    bl_description = "['file_name']カスタムプロパティを追加します"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        context.object["file_name"] = ""
        return {'FINISHED'}
    



#自作クラスまとめ
classes = (MYADDON_OT_stretch_vertex, MYADDON_OT_create_ico_sphere, MYADDON_OT_export_scene, MYADDON_OT_add_filename,
            TOPBAR_MT_my_menu, OBJECT_PT_file_name,
              OBJECT_PT_collider, MYADDON_OT_add_collider)

#Menu描画関数
def draw_menu_manual(self, context):
    #self : 呼び出し元のクラスインスタンス。C++でいうthis
    #context : カーソルを合わせたときのポップアップのカスタマイズなどに使用
    self.layout.operator("wm.url_open_preset", text="Manual", icon='HELP')

#アドオン有効化時コールバック
def register():
    # Blenderにクラスを登録
    for cls in classes:
        bpy.utils.register_class(cls)
        
    #メニューに項目を追加
    bpy.types.TOPBAR_MT_editor_menus.append(TOPBAR_MT_my_menu.submenu)

    DrawCollider.handle = bpy.types.SpaceView3D.draw_handler_add(DrawCollider.draw_collider, (), 'WINDOW', 'POST_VIEW')

    print("レベルエディタが有効化されました。")

#アドオン無効化時コールバック
def unregister():
    bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_my_menu.submenu)

    bpy.types.SpaceView3D.draw_handler_remove(DrawCollider.handle, 'WINDOW')

    for cls in classes:
        bpy.utils.unregister_class(cls)

    print("レベルエディタが無効化されました。")

if __name__ == "__main__":
    register()
