import bpy

from .collider import MYADDON_OT_add_collider

class OBJECT_PT_collider(bpy.types.Panel):
    bl_idname = "OBJECT_PT_collider"
    bl_label = "Collider"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    def draw(self, context):
        if "collider" in context.object:
            #すでにプロパティがあればプロパティを表示
            self.layout.prop(context.object, '["collider"]', text="Collider Type")
            self.layout.prop(context.object, '["collider_center"]', text="Center")
            self.layout.prop(context.object, '["collider_size"]', text="Size")
        else:
            #プロパティがなければ追加するボタンを表示
            self.layout.operator(MYADDON_OT_add_collider.bl_idname, text="コライダー追加", icon='ADD')
