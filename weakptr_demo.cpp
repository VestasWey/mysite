
namespace
{

    class OwnerClass
    {
        class PreMemberClass
        {
        public:
            explicit PreMemberClass(OwnerClass* owner)
                : owner_(owner)
            {
                // owner->weakptr_factory_晚于pre_member_class_instance_，这里是崩的
                //owner_weakptr_ = owner->GetWeakPtr();
            }

            ~PreMemberClass()
            {
                // owner虽然还在，但是其成员变量已经析构了，这时候如果owner_->OnMemberDestroy()中调用了被析构的
                // 成员变量可能就崩了，不崩的话其执行结果也不一定是所期望的
                owner_->OnMemberDestroy();

                // WeakPtr提供了一种方式去判断owner是否处于析构状态中
                if (owner_weakptr_)
                {
                    owner_weakptr_->OnMemberDestroy();
                }
            }

            void AttachOwnerWeakPtr(base::WeakPtr<OwnerClass> owner_weakptr)
            {
                owner_weakptr_ = owner_weakptr;
            }

        protected:
        private:
            OwnerClass* owner_ = nullptr;
            base::WeakPtr<OwnerClass> owner_weakptr_;
        };

        class TailMemberClass
        {
        public:
            TailMemberClass()
                : str_("PreMemberClass str")
                , int_(10000)
                , view_(new views::View)
                , raw_view_(new views::View)
            {
            }

            ~TailMemberClass()
            {
                view_.reset();
                delete raw_view_;
            }

            void InvokeTailMemberClassFunc()
            {
                LOG(INFO) << str_.c_str();
                LOG(INFO) << int_;
                LOG(INFO) << view_.get();
                raw_view_->GetVisibleBounds();
            }

        protected:
        private:
            int int_;
            std::string str_;
            scoped_ptr<views::View> view_;
            views::View* raw_view_ = nullptr;
        };

    public:
        OwnerClass()
            : pre_member_class_instance_(this)
            , protect_member_("this is a protected string")
            , weakptr_factory_(this)
        {
            pre_member_class_instance_.AttachOwnerWeakPtr(weakptr_factory_.GetWeakPtr());
        }

        ~OwnerClass()
        {
        }

        void OnMemberDestroy()
        {
            LOG(INFO) << protect_member_.c_str();

            tail_member_class_instance_.InvokeTailMemberClassFunc();
        }

        base::WeakPtr<OwnerClass> GetWeakPtr()
        {
            return weakptr_factory_.GetWeakPtr();
        }

    private:
        std::string protect_member_;
        PreMemberClass pre_member_class_instance_;
        TailMemberClass tail_member_class_instance_;
        base::WeakPtrFactory<OwnerClass> weakptr_factory_;
    };
}
