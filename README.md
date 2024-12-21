该项目由本人完全独立制作，实现了一款混合了原神与塞尔达风格的游戏：其中启动界面UI，战斗系统和敌人AInpc由蓝图模拟，风格偏向于原神；探索系统则由C++实现，风格偏向于塞尔达。
启动界面UI：
支持进入/退出游戏，风格接近原神。
战斗系统：
支持人物视角转换，多种人物形态转换，攻击，连击；
敌人AInpc系统：
敌人随机巡逻，智能识别玩家，进行攻击，释放冲撞技能；可形成完整的交战场景。
探索系统：
支持人物视角转换，多种人物形态转换，风之翼，风场（可推动风之翼）；
拥有一套适配于探索的技能，模拟塞尔达，包括：
圆形/方形炸弹，爆炸可产生风场，与场景中的植被交互
引力，可牵引特定材质的物品，适用于解谜功能
生成冰柱，可在特定的水面材质上生成随时间变化的冰柱，适用于探险时的水面行走
时间暂停，可暂停某一物体的运动，并随后对它施加方向力，使其被击飞
拥有多个地图
场景交互系统：
可与场景中的特定物品产生拾取，投掷，踩踏压力板等交互，以及炸弹摧毁特定材质障碍，适用于探险系统
音乐系统：在场景中自动播放音乐

该项目的实现，完全是我本人自我学习网课，深耕无数日夜的结果，并未得到他人的任何帮助；在我的努力下，实现了最主要的玩家操作，敌人AI，战斗系统等，并配置了技能系统与场景交互等系统。
在开发的过程中，我一手操办从底层代码到人物动画等的制作，不可谓不鞠躬尽瘁；特别是在UE5中书写C++，几乎是新学了一门比C++复杂得多的语；最终代码量两千五百行（2.5k），还包括巨量的蓝图，亦是我呕心沥血之旅的证明。希望助教老师在看到这一行字时，能体会到个中辛酸，
客观认识到此项巨型工程之不易，给予鄙人公正之评价，鄙人在此再三顿首，以谢师恩。